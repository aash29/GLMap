(define (problem logisticsProblem0)
(:domain logistics_PDDL)
(:objects
	area0 area1 - area
	food1 food2 food3 - food
	door1-0 - door
	key1-0 - key-type
	)

(:init
(npc-at area0)
(npc-not-close-to-point)

(waypoint door1-0)
(point-of-interest door1-0 area1)
(point-of-interest door1-0 area0)
(connected area1 area0 door1-0)
(connected area0 area1 door1-0)
(closed door1-0)

(point-of-interest key1-0 area0)
(item key1-0)
(key key1-0 door1-0)

(item food1)
(item food2)
(item food3)
;(food food1)

(point-of-interest food1 area1)
(point-of-interest food2 area0)
(point-of-interest food3 area0)
)

(:goal
;(and (npc-holding food1) (npc-holding food2))
(forall (?f - food) (npc-holding ?f))
;(forall (?p - passenger) (served ?p)))


;(open door1-0)
;(npc-at area1)
)

)
