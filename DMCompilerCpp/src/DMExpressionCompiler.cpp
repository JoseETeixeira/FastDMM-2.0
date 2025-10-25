#include "DMExpressionCompiler.h"
#include "DMCompiler.h"
#include "DMObjectTree.h"
#include "DreamProcOpcode.h"
#include <iostream>
#include <typeinfo>

namespace DMCompiler {

DMExpressionCompiler::DMExpressionCompiler(DMCompiler* compiler, DMProc* proc, BytecodeWriter* writer)
    : Compiler_(compiler), Proc_(proc), Writer_(writer) {
}

bool DMExpressionCompiler::CompileExpression(DMASTExpression* expr) {
    if (!expr) {
        return false;
    }
    
    // Dispatch based on expression type
    if (auto* constInt = dynamic_cast<DMASTConstantInteger*>(expr)) {
        return CompileConstantInteger(constInt);
    }
    else if (auto* constFloat = dynamic_cast<DMASTConstantFloat*>(expr)) {
        return CompileConstantFloat(constFloat);
    }
    else if (auto* constStr = dynamic_cast<DMASTConstantString*>(expr)) {
        return CompileConstantString(constStr);
    }
    else if (auto* constNull = dynamic_cast<DMASTConstantNull*>(expr)) {
        return CompileConstantNull(constNull);
    }
    else if (auto* constPath = dynamic_cast<DMASTConstantPath*>(expr)) {
        return CompileConstantPath(constPath);
    }
    else if (auto* binOp = dynamic_cast<DMASTExpressionBinary*>(expr)) {
        return CompileBinaryOp(binOp);
    }
    else if (auto* unaryOp = dynamic_cast<DMASTExpressionUnary*>(expr)) {
        return CompileUnaryOp(unaryOp);
    }
    else if (auto* ident = dynamic_cast<DMASTIdentifier*>(expr)) {
        return CompileIdentifier(ident);
    }
    else if (auto* deref = dynamic_cast<DMASTDereference*>(expr)) {
        return CompileDereference(deref);
    }
    else if (auto* call = dynamic_cast<DMASTCall*>(expr)) {
        return CompileCall(call);
    }
    else if (auto* list = dynamic_cast<DMASTList*>(expr)) {
        return CompileList(list);
    }
    else if (auto* newlist = dynamic_cast<DMASTNewList*>(expr)) {
        return CompileNewList(newlist);
    }
    else if (auto* ternary = dynamic_cast<DMASTTernary*>(expr)) {
        return CompileTernary(ternary);
    }
    else if (auto* assign = dynamic_cast<DMASTAssign*>(expr)) {
        return CompileAssign(assign);
    }
    else if (auto* newPath = dynamic_cast<DMASTNewPath*>(expr)) {
        return CompileNewPath(newPath);
    }
    
    // Unsupported expression type
    Compiler_->ForcedWarning("Unsupported expression type for compilation");
    return false;
}

bool DMExpressionCompiler::CompileConstantInteger(DMASTConstantInteger* expr) {
    // Push integer as float (DM treats most numbers as floats)
    Writer_->EmitFloat(DreamProcOpcode::PushFloat, static_cast<float>(expr->Value));
    Writer_->ResizeStack(1);  // Pushes 1 value onto stack
    return true;
}

bool DMExpressionCompiler::CompileConstantFloat(DMASTConstantFloat* expr) {
    Writer_->EmitFloat(DreamProcOpcode::PushFloat, expr->Value);
    Writer_->ResizeStack(1);  // Pushes 1 value onto stack
    return true;
}

bool DMExpressionCompiler::CompileConstantString(DMASTConstantString* expr) {
    Writer_->EmitString(DreamProcOpcode::PushString, expr->Value);
    Writer_->ResizeStack(1);  // Pushes 1 value onto stack
    return true;
}

bool DMExpressionCompiler::CompileConstantNull(DMASTConstantNull* expr) {
    Writer_->Emit(DreamProcOpcode::PushNull);
    Writer_->ResizeStack(1);  // Pushes 1 value onto stack
    return true;
}

bool DMExpressionCompiler::CompileConstantPath(DMASTConstantPath* expr) {
    // The path is already a DreamPath in expr->Path.Path
    const DreamPath& dreamPath = expr->Path.Path;
    
    // Strategy 1: Try direct resolution with context first
    DMObject* context = (Proc_ && Proc_->OwningObject) ? Proc_->OwningObject : nullptr;
    DMObject* typeObj = Compiler_->GetObjectTree()->GetType(dreamPath, context);
    
    if (typeObj != nullptr) {
        // Emit PushType opcode with the type ID
        Writer_->EmitInt(DreamProcOpcode::PushType, typeObj->Id);
        Writer_->ResizeStack(1);  // Pushes 1 value onto stack
        return true;
    }
    
    // Strategy 2: If path is not absolute, try absolute path resolution
    if (dreamPath.GetPathType() != DreamPath::PathType::Absolute) {
        DreamPath absolutePath(DreamPath::PathType::Absolute, dreamPath.GetElements());
        typeObj = Compiler_->GetObjectTree()->GetType(absolutePath, nullptr);
        
        if (typeObj != nullptr) {
            // Emit PushType opcode with the type ID
            Writer_->EmitInt(DreamProcOpcode::PushType, typeObj->Id);
            Writer_->ResizeStack(1);  // Pushes 1 value onto stack
            return true;
        }
    }
    
    // Strategy 3: Try resolution from root context as last resort
    DMObject* root = Compiler_->GetObjectTree()->GetRoot();
    if (root && root != context) {
        typeObj = Compiler_->GetObjectTree()->GetType(dreamPath, root);
        
        if (typeObj != nullptr) {
            // Emit PushType opcode with the type ID
            Writer_->EmitInt(DreamProcOpcode::PushType, typeObj->Id);
            Writer_->ResizeStack(1);  // Pushes 1 value onto stack
            return true;
        }
    }
    
    // All strategies failed - type not found
    // Provide comprehensive error message with resolution context
    if (Compiler_) {
        std::string pathStr = dreamPath.ToString();
        std::string contextMsg = "Type path '" + pathStr + "' could not be resolved\n";
        contextMsg += "  Location: " + expr->Location_.ToString() + "\n";
        
        // Include proc and object context
        if (Proc_) {
            contextMsg += "  Context: proc " + Proc_->Name;
            if (context) {
                contextMsg += " in " + context->Path.ToString();
            }
            contextMsg += "\n";
        }
        
        // Show which resolution strategies were attempted
        contextMsg += "  Resolution attempts:\n";
        contextMsg += "    1. Direct resolution with context";
        if (context) {
            contextMsg += " (" + context->Path.ToString() + ")";
        }
        contextMsg += " - FAILED\n";
        
        if (dreamPath.GetPathType() != DreamPath::PathType::Absolute) {
            DreamPath absolutePath(DreamPath::PathType::Absolute, dreamPath.GetElements());
            contextMsg += "    2. Absolute path resolution (" + absolutePath.ToString() + ") - FAILED\n";
        }
        
        if (root && root != context) {
            contextMsg += "    3. Resolution from root context (/) - FAILED\n";
        }
        
        // Provide suggestions for common issues
        contextMsg += "  Suggestions:\n";
        contextMsg += "    - Verify the type exists in the object tree\n";
        contextMsg += "    - Check if the type is defined before this usage\n";
        if (dreamPath.GetPathType() != DreamPath::PathType::Absolute) {
            contextMsg += "    - Try using an absolute path (e.g., /" + pathStr + ")\n";
        }
        
        Compiler_->ForcedError(expr->Location_, contextMsg);
    }
    return false;
}

bool DMExpressionCompiler::CompileBinaryOp(DMASTExpressionBinary* expr) {
    // Compile left operand (pushes value on stack)
    if (!CompileExpression(expr->Left.get())) {
        return false;
    }
    
    // Compile right operand (pushes value on stack)
    if (!CompileExpression(expr->Right.get())) {
        return false;
    }
    
    // Emit the operation (consumes two values, pushes result)
    DreamProcOpcode opcode = GetBinaryOpcode(expr->Operator);
    if (opcode == DreamProcOpcode::Error) {
        Compiler_->ForcedWarning("Unsupported binary operator");
        return false;
    }
    
    Writer_->Emit(opcode);
    Writer_->ResizeStack(-1);  // Pops 2 values, pushes 1 (net -1)
    return true;
}

bool DMExpressionCompiler::CompileUnaryOp(DMASTExpressionUnary* expr) {
    // Handle increment/decrement operators (they need special handling for references)
    switch (expr->Operator) {
        case UnaryOperator::PreIncrement:
        case UnaryOperator::PreDecrement:
        case UnaryOperator::PostIncrement:
        case UnaryOperator::PostDecrement:
            return CompileIncrementDecrement(expr);
        default:
            break;  // Continue to regular unary operators
    }
    
    // Compile operand
    if (!CompileExpression(expr->Expression.get())) {
        return false;
    }
    
    // Emit unary operation (pops 1, pushes 1, net 0)
    switch (expr->Operator) {
        case UnaryOperator::Negate:
            Writer_->Emit(DreamProcOpcode::Negate);
            return true;
            
        case UnaryOperator::Not:
            Writer_->Emit(DreamProcOpcode::BooleanNot);
            return true;
            
        case UnaryOperator::BitNot:
            Writer_->Emit(DreamProcOpcode::BitNot);
            return true;
            
        default:
            Compiler_->ForcedWarning("Unsupported unary operator");
            return false;
    }
}

bool DMExpressionCompiler::CompileIdentifier(DMASTIdentifier* expr) {
    const std::string& name = expr->Identifier;
    
    // Check if it's a local variable or parameter
    const LocalVariable* localVar = Proc_->GetLocalVariable(name);
    if (localVar) {
        // Emit: PushReferenceValue <RefType.Local> <VariableId>
        std::vector<uint8_t> ref = { 28, static_cast<uint8_t>(localVar->Id) };  // 28 = DMReference.Type.Local
        Writer_->EmitMulti(DreamProcOpcode::PushReferenceValue, ref);
        Writer_->ResizeStack(1);  // Pushes 1 value onto stack
        return true;
    }
    
    // Check for special identifiers (src, usr, args, world, etc.)
    if (name == "src") {
        std::vector<uint8_t> ref = { 1 };  // DMReference.Type.Src
        Writer_->EmitMulti(DreamProcOpcode::PushReferenceValue, ref);
        Writer_->ResizeStack(1);  // Pushes 1 value onto stack
        return true;
    }
    else if (name == "usr") {
        std::vector<uint8_t> ref = { 3 };  // DMReference.Type.Usr
        Writer_->EmitMulti(DreamProcOpcode::PushReferenceValue, ref);
        Writer_->ResizeStack(1);  // Pushes 1 value onto stack
        return true;
    }
    else if (name == "args") {
        std::vector<uint8_t> ref = { 4 };  // DMReference.Type.Args
        Writer_->EmitMulti(DreamProcOpcode::PushReferenceValue, ref);
        Writer_->ResizeStack(1);  // Pushes 1 value onto stack
        return true;
    }
    else if (name == "world") {
        std::vector<uint8_t> ref = { 5 };  // DMReference.Type.World
        Writer_->EmitMulti(DreamProcOpcode::PushReferenceValue, ref);
        Writer_->ResizeStack(1);  // Pushes 1 value onto stack
        return true;
    }
    
    // Check if it's a member variable of the owning object
    if (Proc_->OwningObject) {
        DMVariable* memberVar = Proc_->OwningObject->GetVariable(name);
        if (memberVar) {
            // Push src (the current object)
            std::vector<uint8_t> srcRef = { 1 };  // DMReference.Type.Src
            Writer_->EmitMulti(DreamProcOpcode::PushReferenceValue, srcRef);
            Writer_->ResizeStack(1);  // Pushes src
            
            // Access the field (pops object, pushes field value, net 0)
            int stringId = Compiler_->GetObjectTree()->AddString(name);
            std::vector<uint8_t> fieldData;
            fieldData.push_back(stringId & 0xFF);
            fieldData.push_back((stringId >> 8) & 0xFF);
            fieldData.push_back((stringId >> 16) & 0xFF);
            fieldData.push_back((stringId >> 24) & 0xFF);
            Writer_->EmitMulti(DreamProcOpcode::DereferenceField, fieldData);
            // Net result: 1 value on stack (the field value)
            return true;
        }
    }
    
    // Check if this is a global variable
    const std::vector<DMVariable>& globals = Compiler_->GetObjectTree()->Globals;
    int globalId = Compiler_->GetObjectTree()->GetGlobalVariableId(name);
    if (globalId != -1) {
        // Found it as a global variable - emit reference to global ID
        Writer_->EmitInt(DreamProcOpcode::PushGlobalVars, globalId);
        Writer_->ResizeStack(1);  // Pushes 1 value onto stack
        return true;
    }
    
    // TODO: Check if it's a global proc by name
    // Global procs can be called directly: myproc() instead of global.myproc()
    
    // Unknown identifier - emit detailed warning and push null
    // This matches C# behavior: identifiers that aren't variables/fields/globals
    // become UnknownReference expressions
    std::string contextMsg = "Unknown identifier '" + name + "' at " + expr->Location_.ToString();
    if (Proc_ && Proc_->OwningObject) {
        contextMsg += " in proc " + Proc_->OwningObject->Path.ToString() + "/" + Proc_->Name;
    }
    contextMsg += " (not found as local variable, object field, or global variable)";
    Compiler_->ForcedWarning(contextMsg);
    Writer_->Emit(DreamProcOpcode::PushNull);
    Writer_->ResizeStack(1);  // Pushes 1 value onto stack
    return true;  // Successfully emitted PushNull for unknown identifier
}

DreamProcOpcode DMExpressionCompiler::GetBinaryOpcode(BinaryOperator op) {
    switch (op) {
        // Arithmetic
        case BinaryOperator::Add:
            return DreamProcOpcode::Add;
        case BinaryOperator::Subtract:
            return DreamProcOpcode::Subtract;
        case BinaryOperator::Multiply:
            return DreamProcOpcode::Multiply;
        case BinaryOperator::Divide:
            return DreamProcOpcode::Divide;
        case BinaryOperator::Modulo:
            return DreamProcOpcode::Modulus;
        case BinaryOperator::Power:
            return DreamProcOpcode::Power;
            
        // Comparison
        case BinaryOperator::Equal:
            return DreamProcOpcode::CompareEquals;
        case BinaryOperator::NotEqual:
            return DreamProcOpcode::CompareNotEquals;
        case BinaryOperator::Less:
            return DreamProcOpcode::CompareLessThan;
        case BinaryOperator::Greater:
            return DreamProcOpcode::CompareGreaterThan;
        case BinaryOperator::LessOrEqual:
            return DreamProcOpcode::CompareLessThanOrEqual;
        case BinaryOperator::GreaterOrEqual:
            return DreamProcOpcode::CompareGreaterThanOrEqual;
            
        // Logical
        case BinaryOperator::LogicalAnd:
            return DreamProcOpcode::BooleanAnd;
        case BinaryOperator::LogicalOr:
            return DreamProcOpcode::BooleanOr;
            
        // Bitwise
        case BinaryOperator::BitwiseAnd:
            return DreamProcOpcode::BitAnd;
        case BinaryOperator::BitwiseOr:
            return DreamProcOpcode::BitOr;
        case BinaryOperator::BitwiseXor:
            return DreamProcOpcode::BitXor;
        case BinaryOperator::LeftShift:
            return DreamProcOpcode::BitShiftLeft;
        case BinaryOperator::RightShift:
            return DreamProcOpcode::BitShiftRight;
            
        // DM-specific
        case BinaryOperator::In:
            return DreamProcOpcode::IsInList;
            
        default:
            return DreamProcOpcode::Error;
    }
}

bool DMExpressionCompiler::CompileDereference(DMASTDereference* expr) {
    // Compile the object/list expression first (pushes onto stack)
    if (!CompileExpression(expr->Expression.get())) {
        return false;
    }
    
    // Check if this is field access (obj.field) or indexing (list[index])
    // Field access: Property is an identifier
    // Indexing: Property is any other expression (e.g., integer, variable)
    auto* propIdent = dynamic_cast<DMASTIdentifier*>(expr->Property.get());
    
    if (propIdent) {
        // Field access: obj.field
        // Emit DereferenceField opcode with field name (pops object, pushes field value, net 0)
        Writer_->EmitString(DreamProcOpcode::DereferenceField, propIdent->Identifier);
        // Net result: 1 value on stack (the field value)
    }
    else {
        // Indexing: list[index] or obj[expr]
        // Compile the index expression (pushes index onto stack)
        if (!CompileExpression(expr->Property.get())) {
            return false;
        }
        
        // Emit DereferenceIndex opcode (no operands, uses stack values)
        // Stack: [object/list] [index] -> [value at index]
        Writer_->Emit(DreamProcOpcode::DereferenceIndex);
        Writer_->ResizeStack(-1);  // Pops 2 values (object + index), pushes 1 (result), net -1
    }
    
    return true;
}

bool DMExpressionCompiler::CompileCall(DMASTCall* expr) {
    // For now, only support simple calls with positional arguments
    // Named arguments (key-value pairs) will be added later
    
    // Check for super proc call (..)
    auto* superIdent = dynamic_cast<DMASTIdentifier*>(expr->Target.get());
    if (superIdent && superIdent->Identifier == "..") {
        // Super proc call: ..()
        // Compile arguments first
        int argCount = 0;
        for (const auto& param : expr->Parameters) {
            if (param->Key) {
                std::cerr << "Error: Named arguments not yet supported" << std::endl;
                return false;
            }
            if (!CompileExpression(param->Value.get())) {
                return false;
            }
            argCount++;
        }
        
        // Emit CallStatement opcode with SuperProc reference
        // SuperProc reference type is 7
        std::vector<uint8_t> superProcRef = { 7 };  // DMReference.Type.SuperProc
        
        DMCallArgumentsType argsType = (argCount == 0) ? 
            DMCallArgumentsType::None : DMCallArgumentsType::FromStack;
        
        Writer_->EmitMulti(DreamProcOpcode::CallStatement, superProcRef);
        Writer_->AppendByte(static_cast<uint8_t>(argsType));
        Writer_->AppendInt(argCount);
        
        // CallStatement pops arguments and pushes result (net: 1 - argCount)
        Writer_->ResizeStack(1 - argCount);
        
        return true;
    }
    
    // Check if target is a method call (obj.method()) - DMASTDereference
    auto* deref = dynamic_cast<DMASTDereference*>(expr->Target.get());
    if (deref) {
        // Method call: obj.method(args)
        // Compile object expression
        if (!CompileExpression(deref->Expression.get())) {
            return false;
        }
        
        // Get method name
        auto* methodIdent = dynamic_cast<DMASTIdentifier*>(deref->Property.get());
        if (!methodIdent) {
            std::cerr << "Error: Method name must be an identifier" << std::endl;
            return false;
        }
        
        // Compile arguments (push them onto stack)
        int argCount = 0;
        for (const auto& param : expr->Parameters) {
            if (param->Key) {
                std::cerr << "Error: Named arguments not yet supported" << std::endl;
                return false;
            }
            if (!CompileExpression(param->Value.get())) {
                return false;
            }
            argCount++;
        }
        
        // Emit DereferenceCall: opcode + procName + argumentsType + argumentCount
        DMCallArgumentsType argsType = (argCount == 0) ? DMCallArgumentsType::None : DMCallArgumentsType::FromStack;
        
        // DereferenceCall takes: string (proc name), byte (args type), int (arg count)
        Writer_->EmitString(DreamProcOpcode::DereferenceCall, methodIdent->Identifier);
        Writer_->AppendByte(static_cast<uint8_t>(argsType));
        Writer_->AppendInt(argCount);
        
        // DereferenceCall pops object + arguments, pushes result (net: 1 - argCount - 1 = -argCount)
        Writer_->ResizeStack(-argCount);
        
        return true;
    }
    
    // Global proc call: proc(args)
    // Target should be an identifier (proc name)
    auto* procIdent = dynamic_cast<DMASTIdentifier*>(expr->Target.get());
    if (procIdent) {
        const std::string& procName = procIdent->Identifier;
        
        // Check for built-in functions first
        if (procName == "locate") {
            return CompileLocate(expr);
        } else if (procName == "pick") {
            return CompilePick(expr);
        } else if (procName == "input") {
            return CompileInput(expr);
        } else if (procName == "rgb") {
            return CompileRgb(expr);
        } else if (procName == "prob") {
            return CompileProb(expr);
        } else if (procName == "istype") {
            return CompileIsType(expr);
        } else if (procName == "get_dir") {
            return CompileGetDir(expr);
        } else if (procName == "get_step") {
            return CompileGetStep(expr);
        } else if (procName == "length") {
            return CompileLength(expr);
        } else if (procName == "sqrt") {
            return CompileSqrt(expr);
        }
        
        // Use the improved GetProc() method which searches:
        // 1. Current object and parent chain (if we have an owning object)
        // 2. Global procs
        DMProc* resolvedProc = nullptr;
        int procId = -1;
        
        if (Proc_->OwningObject) {
            // Try to resolve through object hierarchy first, then global procs
            resolvedProc = Compiler_->GetObjectTree()->GetProc(Proc_->OwningObject, procName);
            if (resolvedProc) {
                procId = resolvedProc->Id;
                
                // Check if this is a member proc (not a global proc)
                if (resolvedProc->OwningObject && resolvedProc->OwningObject != Compiler_->GetObjectTree()->GetRoot()) {
                    // This is a member proc call on src (implicit this)
                    // Compile as: push src, call method
                    
                    // Compile arguments first
                    int argCount = 0;
                    for (const auto& param : expr->Parameters) {
                        if (param->Key) {
                            std::cerr << "Error: Named arguments not yet supported" << std::endl;
                            return false;
                        }
                        if (!CompileExpression(param->Value.get())) {
                            return false;
                        }
                        argCount++;
                    }
                    
                    // Push src (the current object)
                    std::vector<uint8_t> srcRef = { 1 };  // DMReference.Type.Src
                    Writer_->EmitMulti(DreamProcOpcode::PushReferenceValue, srcRef);
                    Writer_->ResizeStack(1);  // Pushes src
                    
                    // Call the method
                    DMCallArgumentsType argsType = (argCount == 0) ? 
                        DMCallArgumentsType::None : DMCallArgumentsType::FromStack;
                    
                    Writer_->EmitString(DreamProcOpcode::DereferenceCall, procName);
                    Writer_->AppendByte(static_cast<uint8_t>(argsType));
                    Writer_->AppendInt(argCount);
                    
                    // DereferenceCall pops object + arguments, pushes result (net: 1 - argCount - 1 = -argCount)
                    Writer_->ResizeStack(-argCount);
                    
                    return true;
                }
            }
        } else {
            // No owning object context, try global procs only
            procId = Compiler_->GetObjectTree()->GetGlobalProcId(procName);
        }
        
        if (procId == -1) {
            // Emit warning (not error) for unresolved proc - it may be runtime-defined
            std::string contextMsg = "Unknown proc '" + procName + "()' at " + expr->Location_.ToString();
            if (Proc_ && Proc_->OwningObject) {
                contextMsg += " in proc " + Proc_->OwningObject->Path.ToString() + "/" + Proc_->Name;
            }
            Compiler_->ForcedWarning(contextMsg);
            
            // Emit PushNull as graceful fallback for runtime resolution
            Writer_->Emit(DreamProcOpcode::PushNull);
            Writer_->ResizeStack(1);
            return true;  // Continue compilation despite unresolved proc
        }
        
        // Compile arguments (push them onto stack)
        int argCount = 0;
        for (const auto& param : expr->Parameters) {
            if (param->Key) {
                Compiler_->ForcedWarning("Named arguments not yet supported");
                return false;
            }
            if (!CompileExpression(param->Value.get())) {
                return false;
            }
            argCount++;
        }
        
        // Emit Call opcode with GlobalProc reference
        DMCallArgumentsType argsType = (argCount == 0) ? 
            DMCallArgumentsType::None : DMCallArgumentsType::FromStack;
        
        Writer_->Emit(DreamProcOpcode::Call);
        Writer_->EmitGlobalProcReference(procId);
        Writer_->AppendByte(static_cast<uint8_t>(argsType));
        Writer_->AppendInt(argCount);
        
        // Call pops arguments and pushes result (net: 1 - argCount)
        Writer_->ResizeStack(1 - argCount);
        
        return true;
    }
    
    // Complex call targets (e.g., expressions returning proc references)
    std::cerr << "Error: Complex call targets not yet supported" << std::endl;
    return false;
}

bool DMExpressionCompiler::CompileList(DMASTList* expr) {
    // Compile list creation: list(val1, val2, val3) or []
    // Format: [values pushed onto stack] CreateList <count>
    
    if (expr->IsAssociativeList) {
        std::cerr << "Error: Associative lists (alist) not yet supported" << std::endl;
        return false;
    }
    
    // Compile each value and push onto stack
    int valueCount = 0;
    for (const auto& param : expr->Values) {
        // List values use the same parameter structure as calls
        // Key would be for associative lists (key = value)
        if (param->Key) {
            std::cerr << "Error: Associative list syntax not yet supported" << std::endl;
            return false;
        }
        
        // Compile the value expression
        if (!CompileExpression(param->Value.get())) {
            return false;
        }
        valueCount++;
    }
    
    // Emit CreateList opcode with count
    // Format: CreateList <int count>
    Writer_->Emit(DreamProcOpcode::CreateList);
    Writer_->AppendInt(valueCount);
    
    // CreateList pops valueCount values and pushes 1 list (net: 1 - valueCount)
    Writer_->ResizeStack(1 - valueCount);
    
    return true;
}

bool DMExpressionCompiler::CompileNewList(DMASTNewList* expr) {
    // Compile newlist creation: newlist(/obj/item, /mob/player)
    // This creates a list of new objects by instantiating each type
    // Format: For each parameter:
    //   [push type/value]
    //   CreateObject <DMCallArgumentsType> <StackDelta>
    // Then: CreateList <count>
    
    int objectCount = 0;
    for (const auto& param : expr->Parameters) {
        // Named parameters not supported in newlist
        if (param->Key) {
            std::cerr << "Error: newlist() does not take named arguments" << std::endl;
            return false;
        }
        
        // Compile the parameter expression (typically a type path)
        if (!CompileExpression(param->Value.get())) {
            return false;
        }
        
        // Create an object from the value on the stack
        // CreateObject opcode: <DMCallArgumentsType (byte)> <StackDelta (int32)>
        Writer_->Emit(DreamProcOpcode::CreateObject);
        Writer_->AppendByte(static_cast<uint8_t>(0)); // DMCallArgumentsType::None
        Writer_->AppendInt(0); // StackDelta: 0 arguments
        
        // CreateObject pops type, pushes object (net 0)
        
        objectCount++;
    }
    
    // Create a list from all the created objects
    Writer_->Emit(DreamProcOpcode::CreateList);
    Writer_->AppendInt(objectCount);
    
    // CreateList pops objectCount values and pushes 1 list (net: 1 - objectCount)
    Writer_->ResizeStack(1 - objectCount);
    
    return true;
}

bool DMExpressionCompiler::CompileTernary(DMASTTernary* expr) {
    // Compile ternary operator: condition ? true_expr : false_expr
    // 
    // Bytecode pattern:
    //   [compile condition]
    //   JumpIfFalse false_label
    //   [compile true_expr]
    //   Jump end_label
    // false_label:
    //   [compile false_expr]
    // end_label:
    //   (result is on stack)
    
    // Compile the condition expression (pushes boolean onto stack)
    if (!CompileExpression(expr->Condition.get())) {
        return false;
    }
    
    // Create labels for control flow
    int falseLabel = Writer_->CreateLabel();
    int endLabel = Writer_->CreateLabel();
    
    // If condition is false, jump to false expression
    Writer_->EmitJump(DreamProcOpcode::JumpIfFalse, falseLabel);
    
    // Compile true expression (executed when condition is true)
    if (!CompileExpression(expr->TrueExpression.get())) {
        return false;
    }
    
    // Jump over the false expression to the end
    Writer_->EmitJump(DreamProcOpcode::Jump, endLabel);
    
    // Mark the position for the false expression
    Writer_->MarkLabel(falseLabel);
    
    // Compile false expression (executed when condition is false)
    if (!CompileExpression(expr->FalseExpression.get())) {
        return false;
    }
    
    // Mark the end position (both branches converge here)
    Writer_->MarkLabel(endLabel);
    
    return true;
}

bool DMExpressionCompiler::CompileAssign(DMASTAssign* expr) {
    // Assignment expression: lvalue = rvalue or lvalue op= rvalue
    // Pattern depends on operator type:
    // - Simple assignment (=): push rvalue, then Assign with reference
    // - Compound assignments (+=, -=, *=, /=, etc.): push rvalue, then special opcode with reference
    
    // First, compile the right-hand side (the value to assign)
    if (!CompileExpression(expr->Value.get())) {
        return false;
    }
    
    // Now emit the assignment operation with the left-hand side as a reference
    std::vector<uint8_t> refBytes;
    if (!EmitReference(expr->LValue.get(), refBytes)) {
        std::cerr << "Error: Failed to emit reference for assignment LValue" << std::endl;
        return false;
    }
    
    // Emit the appropriate assignment opcode based on the operator
    switch (expr->Operator) {
        case AssignmentOperator::Assign:
            // Simple assignment: x = y
            Writer_->EmitMulti(DreamProcOpcode::Assign, refBytes);
            break;
            
        case AssignmentOperator::AddAssign:
            // x += y (special opcode: Append)
            Writer_->EmitMulti(DreamProcOpcode::Append, refBytes);
            break;
            
        case AssignmentOperator::SubtractAssign:
            // x -= y (special opcode: Remove)
            Writer_->EmitMulti(DreamProcOpcode::Remove, refBytes);
            break;
            
        case AssignmentOperator::MultiplyAssign:
            // x *= y (special opcode: MultiplyReference)
            Writer_->EmitMulti(DreamProcOpcode::MultiplyReference, refBytes);
            break;
            
        case AssignmentOperator::DivideAssign:
            // x /= y (special opcode: DivideReference)
            Writer_->EmitMulti(DreamProcOpcode::DivideReference, refBytes);
            break;
            
        case AssignmentOperator::ModuloAssign:
            // x %= y (special opcode: ModulusReference)
            Writer_->EmitMulti(DreamProcOpcode::ModulusReference, refBytes);
            break;
            
        case AssignmentOperator::BitwiseAndAssign:
            // x &= y (special opcode: Mask)
            Writer_->EmitMulti(DreamProcOpcode::Mask, refBytes);
            break;
            
        case AssignmentOperator::BitwiseOrAssign:
            // x |= y (special opcode: Combine)
            Writer_->EmitMulti(DreamProcOpcode::Combine, refBytes);
            break;
            
        case AssignmentOperator::BitwiseXorAssign:
            // x ^= y (special opcode: BitXorReference)
            Writer_->EmitMulti(DreamProcOpcode::BitXorReference, refBytes);
            break;
            
        case AssignmentOperator::LeftShiftAssign:
            // x <<= y (special opcode: BitShiftLeftReference)
            Writer_->EmitMulti(DreamProcOpcode::BitShiftLeftReference, refBytes);
            break;
            
        case AssignmentOperator::RightShiftAssign:
            // x >>= y (special opcode: BitShiftRightReference)
            Writer_->EmitMulti(DreamProcOpcode::BitShiftRightReference, refBytes);
            break;
            
        case AssignmentOperator::LogicalAndAssign: {
            // x &&= y means: if (x) { x = y }
            // Bytecode: <ref> JumpIfFalseReference(end) <rhs> Assign <ref> end:
            int endLabel = Writer_->CreateLabel();
            
            // Emit JumpIfFalseReference with reference bytes
            Writer_->EmitJumpWithReference(DreamProcOpcode::JumpIfFalseReference, refBytes, endLabel);
            
            // Emit RHS value
            if (!CompileExpression(expr->Value.get())) {
                std::cerr << "Error: Failed to compile RHS for &&= operator" << std::endl;
                return false;
            }
            
            // Emit assignment
            Writer_->EmitMulti(DreamProcOpcode::Assign, refBytes);
            Writer_->MarkLabel(endLabel);
            break;
        }
            
        case AssignmentOperator::LogicalOrAssign: {
            // x ||= y means: if (!x) { x = y }
            // Bytecode: <ref> JumpIfTrueReference(end) <rhs> Assign <ref> end:
            int endLabel = Writer_->CreateLabel();
            
            // Emit JumpIfTrueReference with reference bytes
            Writer_->EmitJumpWithReference(DreamProcOpcode::JumpIfTrueReference, refBytes, endLabel);
            
            // Emit RHS value
            if (!CompileExpression(expr->Value.get())) {
                std::cerr << "Error: Failed to compile RHS for ||= operator" << std::endl;
                return false;
            }
            
            // Emit assignment
            Writer_->EmitMulti(DreamProcOpcode::Assign, refBytes);
            Writer_->MarkLabel(endLabel);
            break;
        }
            
        case AssignmentOperator::AssignInto:
            // := operator (AssignInto opcode)
            Writer_->EmitMulti(DreamProcOpcode::AssignInto, refBytes);
            break;
            
        default:
            Compiler_->ForcedWarning("Unsupported assignment operator");
            return false;
    }
    
    return true;
}

bool DMExpressionCompiler::EmitReference(DMASTExpression* lvalue, std::vector<uint8_t>& refBytes) {
    // Emit a DMReference for the lvalue expression
    // Supports: identifiers, field access (obj.field), chained field access (a.b.c), indexing (list[index])
    
    if (auto* ident = dynamic_cast<DMASTIdentifier*>(lvalue)) {
        std::string name = ident->Identifier;
        
        // Check for special identifiers
        if (name == "src") {
            refBytes = { 1 };  // DMReference.Type.Src
            return true;
        }
        else if (name == "usr") {
            refBytes = { 3 };  // DMReference.Type.Usr
            return true;
        }
        else if (name == "args") {
            refBytes = { 4 };  // DMReference.Type.Args
            return true;
        }
        else if (name == "world") {
            refBytes = { 5 };  // DMReference.Type.World
            return true;
        }
        
        // Check for local variable first (higher priority than fields)
        LocalVariable* localVar = Proc_->GetLocalVariable(name);
        if (localVar) {
            // Emit DMReference.Type.Local (9) followed by variable ID
            refBytes = { 9, static_cast<uint8_t>(localVar->Id) };
            return true;
        }
        
        // Check for field on the owning object (implicit field access on src)
        // This matches DM semantics: writing "health" in a proc is equivalent to "src.health"
        if (Proc_->OwningObject) {
            const DMVariable* field = Proc_->OwningObject->GetVariable(name);
            if (field) {
                // Emit DMReference.Type.SrcField (11) followed by string ID
                int stringId = Compiler_->GetObjectTree()->AddString(name);
                refBytes = { 11 };
                refBytes.push_back(stringId & 0xFF);
                refBytes.push_back((stringId >> 8) & 0xFF);
                refBytes.push_back((stringId >> 16) & 0xFF);
                refBytes.push_back((stringId >> 24) & 0xFF);
                return true;
            }
        }
        
        // Check for global variables
        int globalId = Compiler_->GetObjectTree()->GetGlobalVariableId(name);
        if (globalId != -1) {
            // Emit DMReference.Type.Global (10) followed by global ID
            refBytes = { 10 };
            refBytes.push_back(globalId & 0xFF);
            refBytes.push_back((globalId >> 8) & 0xFF);
            refBytes.push_back((globalId >> 16) & 0xFF);
            refBytes.push_back((globalId >> 24) & 0xFF);
            return true;
        }
        
        // Variable not found in any scope - provide detailed error
        std::string contextMsg = "Variable '" + name + "' not found for assignment";
        if (Proc_ && Proc_->OwningObject) {
            contextMsg += " in proc " + Proc_->OwningObject->Path.ToString() + "/" + Proc_->Name;
        }
        contextMsg += " (not found as local variable, object field, or global variable)";
        Compiler_->ForcedWarning(contextMsg);
        return false;
    }
    else if (auto* deref = dynamic_cast<DMASTDereference*>(lvalue)) {
        // Field access (e.g., obj.field, src.owner.myBeam) or indexing (list[index])
        // Strategy: Compile the base expression to get the object reference on the stack,
        // then create a reference that dereferences the field/index
        
        // Check if this is field access (obj.field) or indexing (list[index])
        auto* propIdent = dynamic_cast<DMASTIdentifier*>(deref->Property.get());
        
        if (propIdent) {
            // Field access: obj.field or a.b.c
            // Compile the base expression (pushes object onto stack)
            if (!CompileExpression(deref->Expression.get())) {
                return false;
            }
            
            // Create a Field reference (type 12) with the field name
            // DMReference.Type.Field (12) = reference to a field on the object at top of stack
            int stringId = Compiler_->GetObjectTree()->AddString(propIdent->Identifier);
            refBytes = { 12 };  // DMReference.Type.Field
            refBytes.push_back(stringId & 0xFF);
            refBytes.push_back((stringId >> 8) & 0xFF);
            refBytes.push_back((stringId >> 16) & 0xFF);
            refBytes.push_back((stringId >> 24) & 0xFF);
            return true;
        }
        else {
            // Indexing: list[index] or obj[expr]
            // Compile the base expression (pushes list/object onto stack)
            if (!CompileExpression(deref->Expression.get())) {
                return false;
            }
            
            // Compile the index expression (pushes index onto stack)
            if (!CompileExpression(deref->Property.get())) {
                return false;
            }
            
            // Create an Index reference (type 13)
            // DMReference.Type.Index (13) = reference to an indexed element
            // Stack has: [object/list] [index]
            refBytes = { 13 };  // DMReference.Type.Index
            return true;
        }
    }
    
    Compiler_->ForcedWarning("Unsupported LValue type for assignment");
    return false;
}

bool DMExpressionCompiler::CompileIncrementDecrement(DMASTExpressionUnary* expr) {
    // Increment/Decrement operators: ++x, x++, --x, x--
    // These modify a variable in-place and return a value
    //
    // Patterns:
    // - PreIncrement (++x): Push 1, Append reference → returns new value
    // - PostIncrement (x++): Increment reference → returns old value, then increments
    // - PreDecrement (--x): Push 1, Remove reference → returns new value  
    // - PostDecrement (x--): Decrement reference → returns old value, then decrements
    
    // Generate reference for the operand
    std::vector<uint8_t> refBytes;
    if (!EmitReference(expr->Expression.get(), refBytes)) {
        std::cerr << "Error: Failed to emit reference for increment/decrement operand" << std::endl;
        return false;
    }
    
    // Emit appropriate bytecode based on operator type
    switch (expr->Operator) {
        case UnaryOperator::PreIncrement:
            // ++x: push 1, then append (add) to reference
            // This returns the new value (after increment)
            Writer_->EmitFloat(DreamProcOpcode::PushFloat, 1.0f);
            Writer_->ResizeStack(1);  // Pushes 1
            Writer_->EmitMulti(DreamProcOpcode::Append, refBytes);
            // Append pops value, modifies reference, pushes result (net 0)
            break;
            
        case UnaryOperator::PostIncrement:
            // x++: increment reference
            // This returns the old value (before increment)
            Writer_->EmitMulti(DreamProcOpcode::Increment, refBytes);
            Writer_->ResizeStack(1);  // Pushes old value
            break;
            
        case UnaryOperator::PreDecrement:
            // --x: push 1, then remove (subtract) from reference
            // This returns the new value (after decrement)
            Writer_->EmitFloat(DreamProcOpcode::PushFloat, 1.0f);
            Writer_->ResizeStack(1);  // Pushes 1
            Writer_->EmitMulti(DreamProcOpcode::Remove, refBytes);
            // Remove pops value, modifies reference, pushes result (net 0)
            break;
            
        case UnaryOperator::PostDecrement:
            // x--: decrement reference
            // This returns the old value (before decrement)
            Writer_->EmitMulti(DreamProcOpcode::Decrement, refBytes);
            Writer_->ResizeStack(1);  // Pushes old value
            break;
            
        default:
            std::cerr << "Error: Unexpected operator in CompileIncrementDecrement" << std::endl;
            return false;
    }
    
    return true;
}

bool DMExpressionCompiler::CompileNewPath(DMASTNewPath* expr) {
    if (!expr || !expr->Path) {
        std::cerr << "Error: Invalid new expression" << std::endl;
        return false;
    }
    
    // For now, use a placeholder type ID of 0
    // In a full implementation, we would look up the type ID from the object tree
    // using Compiler_->ObjectTree->GetOrCreateObject(expr->Path->Path)
    uint16_t typeId = 0;
    
    // Push the type onto the stack
    Writer_->EmitShort(DreamProcOpcode::PushType, typeId);
    
    // Compile arguments and push them onto the stack
    int argCount = 0;
    for (const auto& param : expr->Parameters) {
        if (!CompileExpression(param->Value.get())) {
            return false;
        }
        argCount++;
    }
    
    // Determine argument type
    DMCallArgumentsType argType = (argCount == 0) ? DMCallArgumentsType::None : DMCallArgumentsType::FromStack;
    
    // Emit CreateObject opcode with argument type and stack size
    std::vector<uint8_t> operands;
    operands.push_back(static_cast<uint8_t>(argType));
    operands.push_back(static_cast<uint8_t>(argCount));
    Writer_->EmitMulti(DreamProcOpcode::CreateObject, operands);
    
    return true;
}

// Built-in function implementations

bool DMExpressionCompiler::CompileLocate(DMASTCall* expr) {
    // locate() can be:
    // 1. locate(type) - locate(type, world)
    // 2. locate(type, container) - search for type in container
    // 3. locate(x, y, z) - get turf at coordinates
    
    size_t argCount = expr->Parameters.size();
    
    if (argCount == 3) {
        // locate(x, y, z) - coordinate lookup
        // Bytecode: Push x, Push y, Push z, LocateCoord
        for (size_t i = 0; i < 3; i++) {
            if (!CompileExpression(expr->Parameters[i]->Value.get())) {
                std::cerr << "Error: Failed to compile locate() coordinate argument " << (i + 1) << std::endl;
                return false;
            }
        }
        Writer_->Emit(DreamProcOpcode::LocateCoord);
        Writer_->ResizeStack(-2);  // Pops 3 values (x, y, z), pushes 1 (turf), net -2
        return true;
    }
    
    if (argCount == 1 || argCount == 2) {
        // locate(type) or locate(type, container)
        // Bytecode: Push type, Push container (or world if omitted), Locate
        
        // Compile type argument
        if (!CompileExpression(expr->Parameters[0]->Value.get())) {
            std::cerr << "Error: Failed to compile locate() type argument" << std::endl;
            return false;
        }
        
        // Compile container argument or push world reference
        if (argCount == 2) {
            if (!CompileExpression(expr->Parameters[1]->Value.get())) {
                std::cerr << "Error: Failed to compile locate() container argument" << std::endl;
                return false;
            }
        } else {
            // Push world reference (global variable)
            // In a full implementation, this would use PushReferenceValue with world reference
            // For now, we'll push null and let the runtime handle it
            Writer_->Emit(DreamProcOpcode::PushNull);
        }
        
        Writer_->Emit(DreamProcOpcode::Locate);
        Writer_->ResizeStack(-1);  // Pops 2 values (type, container), pushes 1 (result), net -1
        return true;
    }
    
    std::string contextMsg = "locate() requires 1, 2, or 3 arguments (found " + std::to_string(argCount) + ") at " + expr->Location_.ToString();
    if (Proc_ && Proc_->OwningObject) {
        contextMsg += " in proc " + Proc_->OwningObject->Path.ToString() + "/" + Proc_->Name;
    }
    Compiler_->ForcedError(expr->Location_, contextMsg);
    return false;
}

bool DMExpressionCompiler::CompilePick(DMASTCall* expr) {
    // pick() can be:
    // 1. pick(val1, val2, ...) - unweighted pick
    // 2. pick(prob(50);val1, prob(30);val2, ...) - weighted pick
    // 
    // For now, we'll only support unweighted pick since detecting
    // weighted syntax requires parsing prob() calls in the parameters
    
    size_t argCount = expr->Parameters.size();
    
    if (argCount == 0) {
        std::string contextMsg = "pick() requires at least 1 argument at " + expr->Location_.ToString();
        if (Proc_ && Proc_->OwningObject) {
            contextMsg += " in proc " + Proc_->OwningObject->Path.ToString() + "/" + Proc_->Name;
        }
        Compiler_->ForcedError(expr->Location_, contextMsg);
        return false;
    }
    
    // Compile all arguments
    for (const auto& param : expr->Parameters) {
        if (!CompileExpression(param->Value.get())) {
            std::cerr << "Error: Failed to compile pick() argument" << std::endl;
            return false;
        }
    }
    
    // For now, always use PickUnweighted
    // Weighted pick detection would require checking if arguments are
    // prob() calls or have semicolon syntax
    std::vector<uint8_t> operands;
    operands.push_back(static_cast<uint8_t>(argCount));
    Writer_->EmitMulti(DreamProcOpcode::PickUnweighted, operands);
    
    // PickUnweighted pops argCount values, pushes 1 (net: 1 - argCount)
    Writer_->ResizeStack(1 - argCount);
    
    return true;
}

bool DMExpressionCompiler::CompileInput(DMASTCall* expr) {
    // input(mob, message, title, default) as type in list
    // The Prompt opcode expects:
    // Stack: default, title, message, mob, list (5 values)
    // Operands: type flags
    
    size_t argCount = expr->Parameters.size();
    
    if (argCount > 4) {
        std::string contextMsg = "input() takes at most 4 positional arguments (found " + std::to_string(argCount) + ") at " + expr->Location_.ToString();
        if (Proc_ && Proc_->OwningObject) {
            contextMsg += " in proc " + Proc_->OwningObject->Path.ToString() + "/" + Proc_->Name;
        }
        Compiler_->ForcedError(expr->Location_, contextMsg);
        return false;
    }
    
    // Push arguments in reverse order (so they're in correct order on stack)
    // Expected order: mob, message, title, default
    // We need to push: default, title, message, mob
    
    // Push arguments 0-3, filling missing ones with null
    // Reverse the order: 3, 2, 1, 0
    for (int i = 3; i >= 0; i--) {
        if (i < static_cast<int>(argCount)) {
            if (!CompileExpression(expr->Parameters[i]->Value.get())) {
                std::cerr << "Error: Failed to compile input() argument " << i << std::endl;
                return false;
            }
        } else {
            Writer_->Emit(DreamProcOpcode::PushNull);
            Writer_->ResizeStack(1);  // Pushes null
        }
    }
    
    // Push list (from "in list" clause, or null if not specified)
    if (expr->InputList) {
        if (!CompileExpression(expr->InputList.get())) {
            std::cerr << "Error: Failed to compile input() list expression" << std::endl;
            return false;
        }
    } else {
        Writer_->Emit(DreamProcOpcode::PushNull);
        Writer_->ResizeStack(1);  // Pushes null
    }
    
    // Prompt opcode takes type flags as operand (from "as type" clause)
    uint32_t typeFlags = static_cast<uint32_t>(expr->InputTypes);
    std::vector<uint8_t> operands;
    operands.push_back(typeFlags & 0xFF);         // Type flags (byte 0)
    operands.push_back((typeFlags >> 8) & 0xFF);  // Type flags (byte 1)
    operands.push_back((typeFlags >> 16) & 0xFF); // Type flags (byte 2)
    operands.push_back((typeFlags >> 24) & 0xFF); // Type flags (byte 3)
    Writer_->EmitMulti(DreamProcOpcode::Prompt, operands);
    
    // Prompt pops 5 values (default, title, message, mob, list), pushes 1 (result), net -4
    Writer_->ResizeStack(-4);
    
    return true;
}

bool DMExpressionCompiler::CompileRgb(DMASTCall* expr) {
    // rgb(r, g, b) or rgb(r, g, b, a) or rgb(x, y, z, space) or rgb(x, y, z, a, space)
    // The Rgb opcode expects arguments on stack and takes arg type + count as operands
    
    size_t argCount = expr->Parameters.size();
    
    if (argCount < 3 || argCount > 5) {
        std::string contextMsg = "rgb() requires 3-5 arguments (found " + std::to_string(argCount) + ") at " + expr->Location_.ToString();
        if (Proc_ && Proc_->OwningObject) {
            contextMsg += " in proc " + Proc_->OwningObject->Path.ToString() + "/" + Proc_->Name;
        }
        Compiler_->ForcedError(expr->Location_, contextMsg);
        return false;
    }
    
    // Compile all arguments
    for (const auto& param : expr->Parameters) {
        if (!CompileExpression(param->Value.get())) {
            std::cerr << "Error: Failed to compile rgb() argument" << std::endl;
            return false;
        }
    }
    
    // Emit Rgb opcode with argument type and count
    DMCallArgumentsType argType = DMCallArgumentsType::FromStack;
    std::vector<uint8_t> operands;
    operands.push_back(static_cast<uint8_t>(argType));
    operands.push_back(static_cast<uint8_t>(argCount));
    Writer_->EmitMulti(DreamProcOpcode::Rgb, operands);
    
    // Rgb pops argCount values, pushes 1 (color), net: 1 - argCount
    Writer_->ResizeStack(1 - static_cast<int>(argCount));
    
    return true;
}

bool DMExpressionCompiler::CompileProb(DMASTCall* expr) {
    // prob(percentage) - returns true with given probability
    // Bytecode: Push percentage, Prob
    
    if (expr->Parameters.size() != 1) {
        std::string contextMsg = "prob() requires exactly 1 argument (found " + std::to_string(expr->Parameters.size()) + ") at " + expr->Location_.ToString();
        if (Proc_ && Proc_->OwningObject) {
            contextMsg += " in proc " + Proc_->OwningObject->Path.ToString() + "/" + Proc_->Name;
        }
        Compiler_->ForcedError(expr->Location_, contextMsg);
        return false;
    }
    
    // Compile the percentage argument
    if (!CompileExpression(expr->Parameters[0]->Value.get())) {
        std::cerr << "Error: Failed to compile prob() argument" << std::endl;
        return false;
    }
    
    // Emit Prob opcode
    Writer_->Emit(DreamProcOpcode::Prob);
    
    // Prob pops 1 value (percentage), pushes 1 (boolean), net 0
    
    return true;
}

bool DMExpressionCompiler::CompileIsType(DMASTCall* expr) {
    // istype(value, type_path) - checks if value is of given type
    // Bytecode: Push value, Push type (via PushType opcode), IsType
    
    if (expr->Parameters.size() != 2) {
        std::string contextMsg = "istype() requires exactly 2 arguments (found " + std::to_string(expr->Parameters.size()) + ") at " + expr->Location_.ToString();
        if (Proc_ && Proc_->OwningObject) {
            contextMsg += " in proc " + Proc_->OwningObject->Path.ToString() + "/" + Proc_->Name;
        }
        Compiler_->ForcedError(expr->Location_, contextMsg);
        return false;
    }
    
    // Compile the value argument (first argument)
    // This pushes the value to check onto the stack
    if (!CompileExpression(expr->Parameters[0]->Value.get())) {
        std::string contextMsg = "Failed to compile istype() value argument at " + expr->Location_.ToString();
        if (Proc_ && Proc_->OwningObject) {
            contextMsg += " in proc " + Proc_->OwningObject->Path.ToString() + "/" + Proc_->Name;
        }
        Compiler_->ForcedError(expr->Location_, contextMsg);
        return false;
    }
    
    // Compile the type argument (second argument)
    // For type path literals (e.g., /atom), this will call CompileConstantPath
    // which now has enhanced resolution strategies and will emit PushType opcode
    if (!CompileExpression(expr->Parameters[1]->Value.get())) {
        std::string contextMsg = "Failed to compile istype() type argument at " + expr->Location_.ToString();
        if (Proc_ && Proc_->OwningObject) {
            contextMsg += " in proc " + Proc_->OwningObject->Path.ToString() + "/" + Proc_->Name;
        }
        contextMsg += "\n  Note: Type path arguments must be valid type paths (e.g., /atom, /mob)";
        Compiler_->ForcedError(expr->Location_, contextMsg);
        return false;
    }
    
    // Emit IsType opcode
    // This opcode expects two values on the stack:
    //   1. The value to check (pushed by first argument)
    //   2. The type to check against (pushed by second argument via PushType)
    Writer_->Emit(DreamProcOpcode::IsType);
    
    // IsType pops 2 values (value, type), pushes 1 (boolean), net -1
    Writer_->ResizeStack(-1);
    
    return true;
}

bool DMExpressionCompiler::CompileGetDir(DMASTCall* expr) {
    // get_dir(loc1, loc2) - returns direction from loc1 to loc2
    // Bytecode: Push loc1, Push loc2, GetDir
    
    if (expr->Parameters.size() != 2) {
        std::cerr << "Error: get_dir() requires exactly 2 arguments (found " << expr->Parameters.size() << ")" << std::endl;
        return false;
    }
    
    // Compile loc1 argument
    if (!CompileExpression(expr->Parameters[0]->Value.get())) {
        std::cerr << "Error: Failed to compile get_dir() loc1 argument" << std::endl;
        return false;
    }
    
    // Compile loc2 argument
    if (!CompileExpression(expr->Parameters[1]->Value.get())) {
        std::cerr << "Error: Failed to compile get_dir() loc2 argument" << std::endl;
        return false;
    }
    
    // Emit GetDir opcode
    Writer_->Emit(DreamProcOpcode::GetDir);
    
    // GetDir pops 2 values (loc1, loc2), pushes 1 (direction), net -1
    Writer_->ResizeStack(-1);
    
    return true;
}

bool DMExpressionCompiler::CompileGetStep(DMASTCall* expr) {
    // get_step(ref, dir) - returns the turf in direction dir from ref
    // Bytecode: Push ref, Push dir, GetStep
    
    if (expr->Parameters.size() != 2) {
        std::cerr << "Error: get_step() requires exactly 2 arguments (found " << expr->Parameters.size() << ")" << std::endl;
        return false;
    }
    
    // Compile ref argument
    if (!CompileExpression(expr->Parameters[0]->Value.get())) {
        std::cerr << "Error: Failed to compile get_step() ref argument" << std::endl;
        return false;
    }
    
    // Compile dir argument
    if (!CompileExpression(expr->Parameters[1]->Value.get())) {
        std::cerr << "Error: Failed to compile get_step() dir argument" << std::endl;
        return false;
    }
    
    // Emit GetStep opcode
    Writer_->Emit(DreamProcOpcode::GetStep);
    
    // GetStep pops 2 values (ref, dir), pushes 1 (turf), net -1
    Writer_->ResizeStack(-1);
    
    return true;
}

bool DMExpressionCompiler::CompileLength(DMASTCall* expr) {
    // length(value) - returns length of text or list
    // Bytecode: Push value, Length
    
    if (expr->Parameters.size() != 1) {
        std::cerr << "Error: length() requires exactly 1 argument (found " << expr->Parameters.size() << ")" << std::endl;
        return false;
    }
    
    // Compile the value argument
    if (!CompileExpression(expr->Parameters[0]->Value.get())) {
        std::cerr << "Error: Failed to compile length() argument" << std::endl;
        return false;
    }
    
    // Emit Length opcode
    Writer_->Emit(DreamProcOpcode::Length);
    
    // Length pops 1 value, pushes 1 (length), net 0
    
    return true;
}

bool DMExpressionCompiler::CompileSqrt(DMASTCall* expr) {
    // sqrt(value) - returns square root of a number
    // Bytecode: Push value, Sqrt
    
    if (expr->Parameters.size() != 1) {
        std::cerr << "Error: sqrt() requires exactly 1 argument (found " << expr->Parameters.size() << ")" << std::endl;
        return false;
    }
    
    // Compile the value argument
    if (!CompileExpression(expr->Parameters[0]->Value.get())) {
        std::cerr << "Error: Failed to compile sqrt() argument" << std::endl;
        return false;
    }
    
    // Emit Sqrt opcode
    Writer_->Emit(DreamProcOpcode::Sqrt);
    
    // Sqrt pops 1 value, pushes 1 (result), net 0
    
    return true;
}

} // namespace DMCompiler


