# How Our Robot Car Works (Explained From Zero)

This file explains every line in `car.ino`. You don't need to know anything about Arduino or coding first. We'll go slowly, one piece at a time.

---

## The Big Idea (the 5-year-old version)

Our robot car is like a bat.

1. It **shouts** a sound that is too high for people to hear.
2. It **listens** for the echo bouncing back.
3. If the echo comes back **fast**, something is **close**, so the car backs up and turns.
4. If the echo comes back **slow**, the path is **clear**, so the car keeps driving forward.

It does this over and over, about 20 times every second, forever.

---

## Our Parts

**Arduino Nano**

This is the brain, and it's what runs the actual code. It's a tiny computer. It can't see or move by itself. It can only turn its **pins** on and off and listen to them.

> **What's a pin?** A little metal leg on the Arduino. Each one has a number (pin 5, pin 6, pin 8...). A wire goes from a pin to another part. The Arduino "talks" to that part by putting electricity on that wire (**ON**) or taking it away (**OFF**).

**Ultrasonic sensor (HC-SR04)**

These are the "eyes" of the car, but it really works like ears. It has two round things that look like eyes:
- One is a **speaker** that sends out a sound (TRIG = "trigger", which means "go!").
- One is a **microphone** that hears the echo (ECHO).

**L298N motor driver**

The Arduino is too weak to spin motors by itself. It's like a kid trying to push a car. The motor driver is the **strong helper**: the Arduino says "spin that way," and the motor driver takes power from the battery and does the heavy lifting.

**Two yellow hobby 6V DC motors (left wheel and right wheel)**

These spin the wheels. "DC" just means they run on battery-style power. "6V" means they're happiest around 6 volts. Each yellow box has a tiny motor plus little gears inside that make the wheel turn slower but stronger.

A DC motor is simple: send power one way and it spins forward. Flip the power around and it spins backward. That's exactly what the motor driver does for us.

**9 volt battery**

This is the **food** for the whole car. It connects to the motor driver, and the motor driver shares that power with the motors (and it can also help power the Arduino).


---

## A Few Coding Rules Before We Start

- **`//` means a note for humans.** The Arduino skips everything after `//` on that line. Those notes are called **comments**.
- **Every instruction ends with `;`**, like a period at the end of a sentence.
- **`{ }` curly braces are a box.** Everything inside them belongs together.
- **The Arduino reads from top to bottom**, one line at a time, super fast.

---

## Part 1: Giving Names to the Pins

**Ultrasonic sensor pins**

```ino
// ---------- Ultrasonic sensor pins ----------
const int trigPin = 5;
const int echoPin = 6;
```

These two lines belong to the ultrasonic sensor, and they tell the Arduino:

    Ultrasonic TRIG -> Arduino pin 5
    Ultrasonic ECHO -> Arduino pin 6

Let's break down one line word by word: `const int trigPin = 5;`

| Word | What it means |
|---|---|
| `const` | "Constant." This value will **never change** while the program runs. It's locked. |
| `int` | "Integer," which is just a whole number (like 5, 10, -3, but **not** 2.5). |
| `trigPin` | The **name** we made up. Now we can say `trigPin` instead of remembering "5". |
| `= 5` | Puts the number 5 into that name. |
| `;` | End of the instruction. |

**Why bother naming it?** Think of it like a name tag. It's easier to read `trigPin` than a random `5`. And if you ever move the wire to a different pin, you only change the number **in one place**.

---

**Motor driver pins**

```ino
// ---------- L298N motor-driver pins ----------
const int IN1 = 8;    // Left motor direction pin 1
const int IN2 = 9;    // Left motor direction pin 2
const int IN3 = 10;   // Right motor direction pin 1
const int IN4 = 11;   // Right motor direction pin 2
```

Same idea. Four wires go from the Arduino to the motor driver:

    Arduino pin 8  -> L298N IN1   (left motor)
    Arduino pin 9  -> L298N IN2   (left motor)
    Arduino pin 10 -> L298N IN3   (right motor)
    Arduino pin 11 -> L298N IN4   (right motor)

Each motor gets **two** wires. Why two? Because the **pair** decides which way the wheel spins:

| Pin A | Pin B | What the wheel does |
|---|---|---|
| ON | OFF | Spins **forward** |
| OFF | ON | Spins **backward** |
| OFF | OFF | **Stops** |

It's like a light switch that has two buttons: one for "go forward," one for "go backward." Press neither and it stops.

> Note: In code, **ON** is written `HIGH` and **OFF** is written `LOW`. You'll see those a lot below.

---

**The "too close" distance**

```ino
// Change this to control how close an object must be
// before the robot avoids it.
const int obstacleDistance = 10;  // centimeters
```

This is the car's **"personal space bubble."** If something is **10 centimeters or closer** (about the length of your hand), the car decides "too close!" and avoids it.

Want the car to be more careful? Make it bigger, like `20`. Want it to get closer before turning? Make it smaller.

---

## Part 2: `setup()`, Getting Ready (runs ONE time)

```ino
void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotors();

  Serial.begin(9600);
}
```

Every Arduino program **must** have a `setup()`. It runs **one time only**, right when you turn the car on. It's like getting dressed before you go outside: you only do it once.

> **What does `void` mean?** It just means "this block does a job but doesn't hand back an answer." Don't worry about it too much yet.

**Line by line:**

`pinMode(trigPin, OUTPUT);`
This tells pin 5: "You are a **talker**." The Arduino will **send** electricity out of this pin (to make the sensor shout).

`pinMode(echoPin, INPUT);`
This tells pin 6: "You are a **listener**." The Arduino will **read** electricity coming in on this pin (to hear the echo).

`pinMode(IN1, OUTPUT);` ... `pinMode(IN4, OUTPUT);`
All four motor pins are **talkers**, because the Arduino gives orders to the motor driver.

> Easy way to remember:
> **OUTPUT** = Arduino **says** something.
> **INPUT** = Arduino **hears** something.

`stopMotors();`
Makes sure the wheels are **not moving** when the car first turns on. Safety first! (We'll see exactly what `stopMotors` does further down.)

`Serial.begin(9600);`
This opens a **walkie-talkie line** between the Arduino and your computer (through the USB cable). Later, the car will use it to tell you how far away things are. `9600` is the talking speed. Your computer has to be set to the same speed (in the Arduino app: **Tools → Serial Monitor**, then pick **9600 baud** in the corner).

---

## Part 3: `loop()`, The Brain's Forever Job (runs AGAIN and AGAIN)

```ino
void loop() {
  int distance = getDistanceCM();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // A zero reading means no echo arrived.
  // Treat it as unsafe and avoid moving forward blindly.
  if (distance == 0 || distance <= obstacleDistance) {
    avoidObstacle();
  } 
  else {
    moveForward();
  }

  delay(50);
}
```

Every Arduino program also **must** have a `loop()`. When it reaches the bottom, it jumps back to the top and starts again. **Forever**, until you unplug it.

It's like a song on repeat.

**Line by line:**

`int distance = getDistanceCM();`
- `getDistanceCM()` is a helper (we'll look inside it later). Its job: **measure how far away the nearest thing is** and give back a number in centimeters.
- `int distance =` makes a box called `distance` and puts that number inside it.
- This one is **not** `const`, because the distance changes every time we measure.

`Serial.print("Distance: ");`
`Serial.print(distance);`
`Serial.println(" cm");`
These send a message over the walkie-talkie line to your computer, like:

    Distance: 42 cm

- `print` writes words and keeps going on the same line.
- `println` (print **line**) writes words and then **starts a new line**, like pressing Enter.
- Words inside `" "` quotes are printed exactly as written. `distance` without quotes prints **the number inside the box**.

This part doesn't move the car at all. It's just so **you** can see what the car sees. Super helpful for fixing problems!

---

**The decision: `if` / `else`**

```ino
if (distance == 0 || distance <= obstacleDistance) {
  avoidObstacle();
} 
else {
  moveForward();
}
```

This is where the car **makes a choice**. It reads like a sentence:

> **IF** the distance is 0, **OR** the distance is 10 or less...
> → then **avoid the obstacle**.
> **OTHERWISE** (else)...
> → **drive forward**.

The symbols:

| Symbol | Means |
|---|---|
| `==` | "is equal to" (two equal signs to **check**; one `=` is for **putting** a value in a box) |
| `\|\|` | "OR" (only one side needs to be true) |
| `<=` | "is less than or equal to" |

**Why is 0 treated as "danger"?** If the sensor shouts and **never hears an echo**, the helper gives back `0`. That could mean the path is wide open... or the sensor is broken or unplugged. We don't know! So the car plays it safe and acts like something is in the way, instead of **driving blind**.

---

`delay(50);`
**Wait 50 milliseconds**, then start the loop again.

> A **millisecond** is 1/1000 of a second. So 50 milliseconds is a tiny blink. That's why the car checks about **20 times every second**.

The short pause gives the old echo time to fade away before the next shout.

---

## Part 4: `getDistanceCM()`, How the Car Measures Distance

```ino
int getDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, 30000UL);

  if (duration == 0) {
    return 0;
  }

  int distance = duration * 0.0343 / 2;

  return distance;
}
```

This is **our own helper** (also called a **function**). We made it so `loop()` can just say `getDistanceCM()` and get an answer, without doing all these steps itself.

This time it starts with `int` instead of `void`. That means: **this helper gives back a whole number** (the distance).

**Step 1: Get quiet**

```ino
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
```
- `digitalWrite(pin, LOW)` = turn that pin **OFF**.
- `delayMicroseconds(2)` = wait **2 microseconds**. (A microsecond is a **millionth** of a second. Crazy fast!)

This makes sure the speaker is quiet before we start, like saying "shhh" before you yell.

**Step 2: SHOUT!**

```ino
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
```
Turn the trigger pin **ON** for 10 microseconds, then **OFF** again. That quick poke tells the sensor: **"Send your sound now!"** The sensor then sends out a burst of sound that humans can't hear.

**Step 3: Listen and time the echo**

```ino
unsigned long duration = pulseIn(echoPin, HIGH, 30000UL);
```
This is like a **stopwatch** .

- The sensor turns the echo pin **ON** while it's waiting for the echo, and **OFF** when the echo comes back.
- `pulseIn(echoPin, HIGH, ...)` measures **how long the echo pin stayed ON**, in microseconds.
- That time goes into a box called `duration`.

The parts of this line:
- `unsigned long` is a type of box for **really big numbers that are never negative**. (Time can't be negative, and the number can get big.)
- `30000UL` is the **give-up time**: 30,000 microseconds, which is 30 milliseconds. If no echo comes back by then, stop waiting. Without this, the car could **freeze** waiting forever. (`UL` just tells the Arduino "this is a big positive number.")

**Step 4: No echo? Give back 0**

```ino
if (duration == 0) {
  return 0;
}
```
If the stopwatch gave up (got 0), then give back **0** right away and skip the rest.

> `return` means "**here's your answer, I'm done**." The helper hands the number back to whoever asked (that's `loop()`).

**Step 5: Turn time into distance**

```ino
int distance = duration * 0.0343 / 2;
return distance;
```
Here's the math, explained simply:

- Sound travels about **0.0343 centimeters every microsecond**.
- So **time × speed = how far the sound went**.
- But the sound went **there AND back** (to the wall and back to the car). We only want the "there" part, so we **divide by 2**.

**Example:** The echo took 580 microseconds.

    580 × 0.0343 = 19.9 cm   (there and back)
    19.9 ÷ 2     = 9.9 cm    (just there)

Since `distance` is an `int` (whole numbers only), the `.9` gets chopped off, so it becomes **9 cm**. That's 10 or less, so... **avoid!**

---

## Part 5: The Movement Helpers

Remember the rule for each motor: **ON + OFF = forward, OFF + ON = backward, OFF + OFF = stop.**

- Left motor = `IN1` and `IN2`
- Right motor = `IN3` and `IN4`

**Move forward**

```ino
// Both motors forward
void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}
```
Left wheel: ON/OFF → forward. Right wheel: ON/OFF → forward.
Both wheels forward = **car goes straight ahead.** ⬆

**Move backward**

```ino
// Both motors backward
void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}
```
Everything is flipped. Both wheels spin backward = **car backs up.** ⬇

**Stop**

```ino
// Stop both motors
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
```
Everything OFF = **no wheels spin.** 

**Turn left**

```ino
// Pivot left: left wheel backward, right wheel forward
void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}
```
Left wheel goes **backward**, right wheel goes **forward**. That makes the car **spin in place** to the left, like a tank or a spinning top.

> Try it with your hands: push your right hand forward and pull your left hand back at the same time. Your body turns left!

**All four moves on one chart:**

| Move | IN1 | IN2 | IN3 | IN4 | Left wheel | Right wheel |
|---|---|---|---|---|---|---|
| Forward | ON | OFF | ON | OFF | forward | forward |
| Backward | OFF | ON | OFF | ON | backward | backward |
| Stop | OFF | OFF | OFF | OFF | stopped | stopped |
| Turn left | OFF | ON | ON | OFF | backward | forward |

> **Tip:** If one wheel spins the wrong way when you test, the code is fine. Just **swap that motor's two wires** on the motor driver.

> **What about speed?** This code has no speed control, so the motors always run at **full speed**. That works because the little plastic caps (called **jumpers**) on the L298N's `ENA` and `ENB` pins are left on.

---

## Part 6: `avoidObstacle()`, The Escape Plan

```ino
// Stop, reverse, then pivot left
void avoidObstacle() {
  stopMotors();
  delay(200);

  moveBackward();
  delay(500);

  stopMotors();
  delay(150);

  turnLeft();
  delay(550);

  stopMotors();
  delay(150);
}
```

This helper uses the other helpers in order, like a little dance routine. 

Here's the most important idea: **the motors keep doing the last thing you told them until you tell them something new.** So `delay()` means "keep doing that for this long."

| Step | Code | What the car does | For how long |
|---|---|---|---|
| 1 | `stopMotors(); delay(200);` | **Freeze!** | 0.2 seconds |
| 2 | `moveBackward(); delay(500);` | **Back up** away from the thing | 0.5 seconds |
| 3 | `stopMotors(); delay(150);` | Short pause | 0.15 seconds |
| 4 | `turnLeft(); delay(550);` | **Spin left** to face a new way | 0.55 seconds |
| 5 | `stopMotors(); delay(150);` | Short pause | 0.15 seconds |

The whole escape takes a little over **1.5 seconds**. Then `avoidObstacle()` is done, `loop()` goes back to the top, and the car **measures again**. If the new path is clear, it drives forward. If not, it does the escape plan again.

**Why the short stops in between?** Flipping a motor from forward to backward instantly is hard on the motors and batteries, and it makes the car jerk around. A tiny pause is gentler.

**Want to turn more or less?** Change `550` in the `turnLeft` step. A bigger number means a bigger turn. A smaller number means a smaller turn. (The exact angle depends on your motors, batteries, and floor, so test it!)

---

## Word List (Glossary)

| Word | Simple meaning |
|---|---|
| **Pin** | A metal leg on the Arduino that a wire connects to |
| **HIGH / LOW** | ON / OFF |
| **OUTPUT / INPUT** | Arduino talks / Arduino listens |
| **Variable** | A labeled box that holds a value (like `distance`) |
| **`const`** | A locked box whose value never changes |
| **`int`** | A whole number |
| **`unsigned long`** | A box for big numbers that are never negative |
| **Function** | A named set of steps you can use over and over (like `moveForward()`) |
| **`void`** | This function does a job but gives nothing back |
| **`return`** | Give an answer back and finish the function |
| **`setup()`** | Runs once at the start |
| **`loop()`** | Runs over and over forever |
| **`delay(ms)`** | Wait this many milliseconds (1000 ms = 1 second) |
| **`delayMicroseconds(us)`** | Wait this many microseconds (1,000,000 us = 1 second) |
| **`digitalWrite`** | Turn a pin ON or OFF |
| **`pulseIn`** | A stopwatch that times how long a pin stays ON |
| **Serial** | The walkie-talkie line to your computer over USB |
| **Comment `//`** | A note for humans that the Arduino ignores |

---

## Things You Can Try Changing

| Change this | In this spot | What happens |
|---|---|---|
| `obstacleDistance = 10` | Top of the file | Bigger = car turns away sooner |
| `delay(500)` after `moveBackward()` | `avoidObstacle()` | Bigger = backs up farther |
| `delay(550)` after `turnLeft()` | `avoidObstacle()` | Bigger = turns more |
| `delay(50)` | Bottom of `loop()` | Smaller = checks more often |

Change **one thing at a time**, upload, and watch what happens. That's how you learn! 
