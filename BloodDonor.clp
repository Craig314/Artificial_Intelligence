;;Craig Hulsebus - CSC180 - 09/27/2018
;;Procedural Rules For Blood Donation Exam

(defrule p1
   ?p <- (start)
   =>
   (printout t "Has the donor finished registering? " crlf "(yes/no): ")
   (assert (registered (read)))
   (retract ?p)
)
(defrule rule1
   (registered no)
   =>
   (printout t "Take donor back to fill out registration forms." crlf )
)
(defrule p2
   (registered yes)
   =>
   (printout t "Has the donor completed the Health History Questionnaire? " crlf "(yes/no): ")
   (assert (history (read)))
)
(defrule rule2
   (history no)
   =>
   (printout t "Take donor back to complete Health History Questionnaire." crlf)
)
(defrule p3
   (history yes)
   =>
   (printout t "Is the donor of age 18 years or older? " crlf)
   (printout t "Note: If donor is age 16 or 17 years old, they may donate if they have a Minor Donor Permission form filled out and signed by a guardian." crlf "(yes/no): ")
   (assert (age (read)))
)
(defrule rule3
   (age no)
   =>
   (printout t "The donor is not old enough to donate." crlf "Let the donor know they must be 18 years or older, or if they are 16 or 17, they must have the Minor Donor Permission Form in order to donate." crlf)
)
(defrule p4
   (age yes)
   =>
   (printout t "Take the donor's weight. " crlf)
   (printout t "Is the donor's weight at least 110 pounds?" crlf "(yes/no): ")
   (assert (weight (read)))
)
(defrule rule4
   (weight no)
   =>
   (printout t "Inform the donor that they are not healthy enough to donate blood." crlf "Let them know that they must weigh at least 110 pounds if they wish to donate in the future." crlf)
)
(defrule p5
   (weight yes)
   =>
   (printout t "Take the donor's pulse." crlf "Does the donor's pulse sound normal? " crlf "(yes/no): ")
   (printout t )
   (assert (pulse (read)))
)
(defrule rule5
   (pulse no)
   =>
   (printout t "Inform the donor of the issue that you have noticed, and do not proceed furthor with the donation." crlf "Let the donor know that, although you are not a registered doctor, it is within your best judgement to not allow the donation." crlf "Reccomend they go see their primary care. In case of emergency, advise them to go to the hospital." crlf)
)

;;Fuzzzy Associative Matrix - Blood Pressure
;;Fuzzy Set Definition
(deftemplate TopNumber
   100 140 degrees
   (  (Low (110 1) (120 0))
      (Good (110 0) (120 1) (130 0))
      (High (120 0) (130 1)))
)

(deftemplate BottomNumber
   60 100 degrees
   (  (Low (70 1) (80 0))
      (Good (70 0) (80 1) (90 0))
      (High (80 0) (90 1))
   )
)
(deftemplate Result
   160 240 degrees
   (  (VeryLow (173.3 1) (186.6 0))
      (Low (173.3 0) (186.6 1) (200 0))
      (Good (186.6 0) (200 1) (213.3 0))
      (High (200 0) (213.3 1) (226.6 0))
      (VeryHigh (213.3 0) (226.6 1))
   )
)
;;Get crisp inputs
(defrule Get_Observations1
   (pulse yes)
   ?i <- (initial-fact)
   =>
   (printout t "Take the donor's blood pressure and enter their results." crlf)
   (printout t "Enter Top Number: " )
   (bind ?response (read))
   (assert (crispTopNumber ?response))
   (printout t "Enter Lower Number: ")
   (bind ?response (read))
   (assert (crispLowerNumber ?response))
)
;;Fuzzify the inputs
(defrule fuzzify1
   (crispTopNumber ?a)
   (crispLowerNumber ?d)
   =>
   (assert (TopNumber (?a 0) (?a 1) (?a 0)))
   (assert (BottomNumber (?d 0) (?d 1) (?d 0)))
)
;;Defuzzify the outputs
(defrule defuzzify1
   (declare (salience -1))
   ?f <- (Result ?)
   =>
   (bind ?t (moment-defuzzify ?f))
   (printout t "Blood Pressure--> " ?t crlf)
   (assert (BP yes))
)
;;FAM rule definition
(defrule HighHigh
   (TopNumber High)
   (BottomNumber High)
   =>
   (assert (Result VeryHigh))
)
(defrule HighGood
   (TopNumber High)
   (BottomNumber Good)
   =>
   (assert (Result High))
)
(defrule HighLow
   (TopNumber High)
   (BottomNumber Low)
   =>
   (assert (Result Good))
)
(defrule GoodHigh
   (TopNumber Good)
   (BottomNumber High)
   =>
   (assert (Result High))
)
(defrule GoodGood
   (TopNumber Good)
   (BottomNumber Good)
   =>
   (assert (Result Good))
)
(defrule GoodLow
   (TopNumber Good)
   (BottomNumber Low)
   =>
   (assert (Result Low))
)
(defrule LowHigh
   (TopNumber Low)
   (BottomNumber Good)
   =>
   (assert (Result Good))
)
(defrule LowGood
   (TopNumber Low)
   (BottomNumber Good)
   =>
   (assert (Result Low))
)
(defrule LowLow
   (TopNumber Low)
   (BottomNumber Low)
   =>
   (assert (Result VeryLow))
)

(defrule rule6
   (BP yes)
   =>
   (printout t "Good: 186.6 - 200  Low: 160 - 186.6 High: 200 - 240 " crlf "If the results are within either the low or high range, use your professional opinion to decide whether to allow the donor to donate or not." crlf)
   (assert (BPP yes))
)

(defrule p6
   (BPP yes)
   =>
   (printout t "Is the blood pressure reading good to proceed?" crlf "(yes/no): ")
   (assert (pressure (read)))
)

(defrule rule7
   (pressure no)
   =>
   (printout t "Do not proceed further." crlf)
)

;;Fuzzy Associative Matrix - Hemoglobin Level and Body Temp
;;Fuzzy Set Definition
(deftemplate Hemoglobin
   12 18 numbers
   (  (L (13.5 1) (15 0))
      (G (13.5 0) (15 1) (16.5 0))
      (H (15 0) (16.5 1))
   )
)
(deftemplate BodyTemp
   97 100 numbers
   (  (L (97.75 1) (98.5 0))
      (G (97.75 0) (98.5 1) (99.25 0))
      (H (99.25 0) (100 1))
   )
)
(deftemplate Result2
   109 118 numbers
   (  (VL (110.5 1) (112 0))
      (L (110.5 0) (112 1) (113.5 0))
      (G (112 0) (113.5 1) (115 0))
      (H (113.5 0) (115 1) (116.5 0))
      (VH (115 0) (116.5 1))
   )
)
;;Get crisp inputs
(defrule Get_Observations2
   (pressure yes)
   ?i <- (initial-fact)
   =>
   (printout t "Take the donor's Hemoglobin levels and their body temperature and enter their results." crlf)
   (printout t "Enter Hemoglobin Levels: ")
   (bind ?response (read))
   (assert (crispHemoglobin ?response))
   (printout t "Enter Body Temperature in F: ")
   (bind ?response (read))
   (assert (crispBodyTemp ?response))
   (retract ?i)
)
;;Fuzzify the inputs
(defrule fuzzify2
   (crispHemoglobin ?a)
   (crispBodyTemp ?d)
   =>
   (assert (Hemoglobin (?a 0) (?a 1) (?a 0)))
   (assert (BodyTemp (?d 0) (?d 1) (?d 0)))
)
;;Defuzzify the outputs
(defrule defuzzify2
   (declare (salience -1))
   ?f <- (Result2 ?)
   =>
   (bind ?t (moment-defuzzify ?f))
   (printout t "Hemoglobin and Temperature--> " ?t crlf)
   (assert (HT yes))
)
;;FAM rule definition
(defrule HH
   (Hemoglobin H)
   (BodyTemp H)
   =>
   (assert (Result2 VH))
)
(defrule HG
   (Hemoglobin H)
   (BodyTemp G)
   =>
   (assert (Result2 H))
)
(defrule HL
   (Hemoglobin H)
   (BodyTemp L)
   =>
   (assert (Result2 G))
)
(defrule GH
   (Hemoglobin G)
   (BodyTemp H)
   =>
   (assert (Result2 H))
)
(defrule GG
   (Hemoglobin G)
   (BodyTemp G)
   =>
   (assert (Result2 G))
)
(defrule GL
   (Hemoglobin G)
   (BodyTemp L)
   =>
   (assert (Result2 L))
)
(defrule LH
   (Hemoglobin L)
   (BodyTemp G)
   =>
   (assert (Result2 G))
)
(defrule LG
   (Hemoglobin L)
   (BodyTemp G)
   =>
   (assert (Result2 L))
)
(defrule LL
   (Hemoglobin L)
   (BodyTemp L)
   =>
   (assert (Result2 VL))
)

(defrule p7
   (HT yes)
   =>
   (printout t "Good: 112 - 115  Low: 109 - 112 High: 115 - 118 " crlf "If both Blood Pressure and Hemoglobin Levels come back within an acceptable range, complete your examination and clear the donor to proceed with a phlebotomist." crlf)
   (assert (HTT yes))
)

(deffacts startup
   (start)
)
