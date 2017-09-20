
(define (domain logistics_PDDL)
(:requirements :adl)
(:types
    food - object
    door - object
    area - object
    key-type - object)

(:predicates 
	(npc-at ?area)
	(npc-close-to ?point)
    (npc-holding ?o)
	(npc-not-close-to-point)

	(point-of-interest ?point ?area)

	(item ?item)

	(key ?item ?waypoint)

	(connected ?area1 ?area2 ?waypoint)
		
	(waypoint ?waypoint)
	(open ?waypoint)
	(closed ?waypoint)
)

(:action move-to-point
:parameters (?area ?point)
:precondition (and
		(npc-at ?area)
		(point-of-interest ?point ?area)
		(npc-not-close-to-point)
		)
:effect (and
		(npc-close-to ?point)
		(not (npc-not-close-to-point))
	)
)


(:action move-to-area
:parameters (?fromarea ?toarea ?waypoint)
:precondition (and 
		(connected ?fromarea ?toarea ?waypoint)
               	(npc-at ?fromarea)
		(waypoint ?waypoint)
		(open ?waypoint)
		(npc-close-to ?waypoint)
		)
:effect (and 
		(npc-at ?toarea)
		(npc-not-close-to-point)
             	(not (npc-at ?fromarea))
		(not (npc-close-to ?waypoint))
	)
)


(:action move-to-point-from-point
:parameters (?area ?point ?previouspoint)
:precondition (and
		(npc-at ?area)
		(point-of-interest ?point ?area)
		(point-of-interest ?previouspoint ?area)
		(npc-close-to ?previouspoint)
		)
:effect (and
		(npc-close-to ?point)
		(not (npc-close-to ?previouspoint))
		(not (npc-not-close-to-point))
	)
)

(:action make-accessible
:parameters (?area1 ?area2 ?waypoint ?item)
:precondition (and
		(npc-holding ?item)
		(key ?item ?waypoint)
		(npc-at ?area1)
		(connected ?area1 ?area2 ?waypoint)
		(waypoint ?waypoint)
		(closed ?waypoint)
		(npc-close-to ?waypoint)
		)
:effect (and
		(not (closed ?waypoint))
		(open ?waypoint)
		(not (npc-holding ?item))
	)
)


(:action place-in-inventory
:parameters (?area ?item)
:precondition (and 
		(npc-at ?area)
               	(point-of-interest ?item ?area)
		(npc-close-to ?item)
		(item ?item)
		)
:effect (and 
		(not (point-of-interest ?item ?area))
             	(npc-holding ?item)
		(not (npc-close-to ?item))
		(npc-not-close-to-point)
	)
)

)
