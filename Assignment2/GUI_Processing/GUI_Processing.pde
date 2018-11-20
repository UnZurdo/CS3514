// Read and write data from the serial port. 

import processing.serial.*; 
 
Serial port;  // Create object from Serial class
int val;      // Data received from the serial port 
String PORT_NAME = "/dev/tty.usbmodem142203";
String myString = null;


class Test {
  //members
  float x;
  float y;
  float w, h;
  float initialY;
  boolean lock = false;
 
  //constructors
 
  //default
  Test () {
  }
 
  Test (float _x, float _y, float _w, float _h) {
    x=_x;
    y=_y;
    initialY = y;
    w=_w;
    h=_h;
  }
 
 
  void run() {
 
    float lowerY = height - h - initialY;
    float value = map(y, initialY, lowerY, 120, 255);
    int value2 = int(map(value, 120, 255, 40, 208)) / 4;
    color c = color(value);
    fill(c);
    rect(x, initialY, 4, lowerY);
    fill(215);
    rect(x, y, w + 15, h);
    fill(0);
    textSize(11);
    text(nf(int(value2 * 4), 3) +" BPM", x+5, y+13);
    float my = constrain(mouseY, initialY, height - h - initialY );
    if (lock) {
      y = my;
      println("Frequency: " + value2 * 4);
      port.write(value2 * 4);

    }
    
    myString = port.readStringUntil(10);
    if (myString != null) {
      println(myString);
    }
    

  
  }
 
 
  boolean isOver()
  {
    return (x+w >= mouseX) && (mouseX >= x) && (y+h >= mouseY) && (mouseY >= y);
  }
}
 

Test instances =  new Test(); 
 
void setup() {
  size(400, 600);
  noStroke(); 
  instances = new Test(20, 20, 40, 15);
  frameRate(10);  // Run 10 frames per second
  // Open the port that the board is connected to and use the same speed (9600 bps) 
  port = new Serial(this, PORT_NAME, 9600); 

}
 
void draw() {
  background(100);
  instances.run();

  rect(100, 50, 100, 100);   // Draw square
  rect(250, 50, 100, 100);         // Draw square

  rect(100, 200, 100, 100);   // Draw square
  rect(250, 200, 100, 100);         // Draw square
  
  
  fill(255);   
  text("START ", 130,100);
  text(" STOP ", 280,100);
  
  text("MIDDLE C ", 125, 250);
  text("CONCERT A", 270, 250);
  
  // while (port.available() > 0) {
  //  myString = port.readStringUntil(10);
  //  if (myString != null) {
  //    println(myString);
  //  }
  //}

}
 
 
void mousePressed() {
    if (instances.isOver()){
      instances.lock = true;
    }
    if (mouseX <= 200 && mouseX >= 100 && mouseY <= 150){
      fill(155);
      rect(100, 50, 100, 100);   // Draw square
      port.write(1);
    }
    if (mouseX <= 350 && mouseX >= 250 && mouseY <= 150){
      fill(155);
      rect(250, 50, 100, 100);         // Draw square
      port.write(0);
    }
    // MIDDLE C
     if (mouseX <= 200 && mouseX >= 100 && mouseY >= 200 && mouseY <= 250 ){
      fill(155);
  rect(100, 200, 100, 100);   // Draw square
      port.write(3);
    }
    // CONCERT A
    if (mouseX <= 350 && mouseX >= 250 && mouseY >= 200 && mouseY <= 250){
      fill(155);
  rect(250, 200, 100, 100);         // Draw square
      port.write(2);
    }
}
 
void mouseReleased() {
  {
    instances.lock = false;
  }
}
