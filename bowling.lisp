;bowling ten frame
;strike = first throw = 10
;spare = first + second = 10
;strike countes 10 plus next two throws
;spare counts 10 plus next throw
;otherwise  frame is sum of two throws
;tenth frame is special - chance for extra throws
;  1    2    3    4    5    6    7    8    9    10
;[10|][5|5][10|][6|3][ | ][ | ][ | ][ | ][ | ][ | , ]
;  20  40   59 ....
;lis => '(10   5 5  10   6 3  10    7 3 9 1   10 8 0  5 5 5)

(define cadr(lis) (car (cdr lis)))
(define cddr(lis) (cdr (cdr lis)))
(define caddr(lis) (car (cddr lis)))
(define cdddr(lis) (cdr (cddr lis)))

(define strike? (lis) (eq 10 (car lis)) )
(define spare? (lis) (eq 10 (sum2 lis)) )
(define sum2(lis)
	(+ (car lis) (cadr lis))
)
(define sum3(lis) 
	(+ (sum2 lis)(caddr lis))
)
(define drop1 (lis) (cdr lis) )
(define drop2 (lis) (cddr lis) )
(define drop (lis) (cond
    ((spare? lis) (drop2 lis))
	((strike? lis) (drop1 lis))
	('t (drop2 lis))
))

(define bowlingscore (lis)
	(bowling lis 1)
)
(define inc (x) (+ 1 x))
(define bowling(lis frame#)(cond
	((eq frame# 11) 0)
	('t (+ (scorethisframe lis)
		(bowling (drop lis) (inc frame#))))
))
(define scorethisframe (lis)(cond
	((or (strike? lis) (spare? lis)) (sum3 lis))
	('t (sum2 lis))
))

(print (bowlingscore '(
    0 0
    0 0
    0 0
    0 0
    0 0
    0 0
    0 0
    0 0
    0 0
    0 0
     )
))
(print (bowlingscore '(
    0 1
    0 1
    0 1
    0 1
    0 1
    0 1
    0 1
    0 1
    0 1
    0 1
     )
))
(print (bowlingscore '(
    0 1;1
    2 3;6
    4 5;15
    6 3;24
    7 2;33
    8 1;42
    9 0;51
    1 8;60
    3 6;69
    5 4;78
     )
))
(print (bowlingscore '(
    0 10
    0 10
    0 10
    0 10
    0 10
    0 10
    0 10
    0 10
    0 10
    0 10 0
     )
))
(print (bowlingscore '(
    10
    10
    10
    10
    10
    10
    10
    10
    10
    10 10 10
     )
))















