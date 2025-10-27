This is the midterm project for CSCI 441 to which we have formed the group THE ETERNAL ALCHEMISTS consisting of members Elyse Franklin, Elijah Booth, Leviathan Douglas, and Stew Nowak.
UPDATE THIS AS WE GO!!
Note I have added the beginning of our "world". I hope the skeleton works/is a good place to begin, but if you have details or implementations you find better from doing your own A3, PLEASE use them! Just comment along and document what you do as we go!

---
Player.hpp

Hopefully I got functions right... feel free to change it if there are any problems.

Has standard movement functions moveForward(speed), moveBackward(speed), and rotate(theta, phi). draw() and animate(dTime) need to be overridden. The constructor should also be overridden with model-loading information.

_computeOrientation() currently doesn't have phi (I broke mine in A3 and forgot to fix it since it was unused).
