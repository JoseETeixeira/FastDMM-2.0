// Combat system

/mob
    var/health = 100
    
    proc/take_damage(amount)
        health -= amount
        if(health <= 0)
            die()
    
    proc/die()
        del(src)

/mob/player
    name = "Player"
    
    New()
        ..()
        world << "Player spawned!"
    
    proc/heal(amount)
        health += amount
        if(health > 100)
            health = 100

/obj/item
    proc/pickup()
        world << "Picked up!"

proc/global_test()
    world << "Global test!"
