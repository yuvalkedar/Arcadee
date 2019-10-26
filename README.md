# Arcadee

Machine Controller For Gigantic LTD.

Generally, the machine shoots basketballs and reloads them.

The user has two buttons in the app:

- First button is responsible for the aiming:
  A long press will make the launcher/canon sweep left and right repeatedly.
  Releasing the button will stop the canon at its current position.

- Second button is responsible for the shooting strength:
  A long press will change the LED strength bar up and down (there are 8 LEDs).
  Releasing the button will determine the shooting strength related to the LED's position.

After releasing the second button, the magazine will load a ball to the canon.
The canon will shoot the ball and then the machine will reset its position, the LED bar, etc. and will reload a ball to the magazine.

What is inside loop()?

The machine lets the user start a game only if there's a loaded ball ready for shooting, and finish the game only when the ball is reloaded.
If the aiming button is pressed aiming_servo.Update() is called.
If the shooting button is pressed strength_timer starts, this timer is responsible for updating the strength.
The machine always checks for winning.

THE ISSUE: Once in a million games, the strength LED bar is stuck on the first LED. The machine's workflow routine isn't harmed; the user can keep playing -
when he will release the button what should happen will happen (the canon will shoot the ball, then the machine will reload the ball, and the game will over).
I use multiple timers, which I'm pretty sure are the reason for this issue.
**_ I added FIXME where you should look at. _**
