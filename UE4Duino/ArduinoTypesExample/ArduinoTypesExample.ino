const int r_input_pin = A0;
const int l_input_pin = A1;

// Define the number of samples to keep track of. The higher the number, the
// more the readings will be smoothed, but the slower the output will respond to
// the input. Using a constant rather than a normal variable lets us use this
// value to determine the size of the readings array.
double filter = 10; // con 15 era parecchio lento a sterzare
float r_value, l_value;
float l_value__calibrated, r_value__calibrated;

void setup()
{
  // init serial communication with computer:
  Serial.begin(9600);
//  Serial.setTimeout(5);

  l_value__calibrated = l_value = analogRead( l_input_pin );
  r_value__calibrated = r_value = analogRead( r_input_pin );

  // la calibrazione la faccio manualmente.
  // la calibrazione automatica ha un problema,
  // ogni volta che riavvio il gioco torna a calibrare
  l_value__calibrated = 866;
  r_value__calibrated = 458;
//  Serial.print( l_value__calibrated ); // Turn
//  Serial.println();
//  Serial.print( r_value__calibrated ); // Turn
//  Serial.println();
}

void loop_debug(){
  Serial.print( analogRead( l_input_pin ) ); // Turn
//  Serial.print( "," );
//  Serial.print( analogRead( r_input_pin ) ); // Turn
  Serial.println();
}
void loop()
{
//  loop_debug();
//  return;

  // leggo costantemente
  fetchDimmerSmoothValues();

  // check if serial requests
  if( ! Serial.available() ) return;

  // parse serial requests
  String command = Serial.readString();
  switch( command ){
    case "read_values":
      sendToSerial();
      break;  
  }
}

void fetchDimmerSmoothValues(){
  // filtered value by: https://github.com/XRobots/openDog/blob/master/Part3/code/Dog003/Dog003.ino#L166

  int l_max_run = 20;
  int r_max_run = 38;

  int l_value_min = l_value__calibrated;
  int l_value_max = l_value_min - l_max_run;
  float l_value_raw = analogRead( l_input_pin );
  float l_value_constrained = constrain( l_value_raw, l_value_max, l_value_min ); // inverted
  float l_value_mapped = map( l_value_constrained, l_value_min, l_value_max, 1000, -1000 );
  l_value_mapped = l_value_mapped;
  l_value = ( l_value_mapped + ( l_value * filter ) ) / ( filter + 1 );

  int r_value_min = r_value__calibrated;
  int r_value_max = r_value_min + r_max_run;
  float r_value_raw = analogRead( r_input_pin );
  float r_value_constrained = constrain( r_value_raw, r_value_min, r_value_max ); // inverted
  float r_value_mapped = map( r_value_constrained, r_value_min, r_value_max, 1000, -1000 );
  r_value = ( r_value_mapped + ( r_value * filter ) ) / ( filter + 1 );
}

void sendToSerial(){
  // from UE4Duino: https://github.com/RVillani/UE4Duino/blob/master/UE4Duino/ArduinoTypesExample/ArduinoTypesExample.ino#L35

  /* remove/add the first slash to toggle the enabled code
    // Serial.println( (int)r_value ); // Right
    // Serial.println( (int)l_value ); // Left
  /*/

  /* LEFT */
  print__l_turn();
  Serial.print( "," );

  /* RIGHT */
  print__r_turn();
  Serial.print( "," );

  /* SPEED (acceler.) */
  print__speed();
  Serial.print( "," );

  /* TURN (steering) */
  print__turn();
  Serial.println();

  /* RAW VALUES */
  if( false ){
    Serial.print( "," );
    Serial.print( analogRead( l_input_pin ) );
    Serial.print( "," );
    Serial.print( analogRead( r_input_pin ) );
  }
}

void print__l_turn(){
  float l_turn = l_value / 1000; // porto a scala -1.00 - 1.00

  Serial.print( l_turn );
}
void print__r_turn(){
  float r_turn = r_value / 1000; // porto a scala -1.00 - 1.00

  Serial.print( r_turn );
}
void print__speed(){
  float speed = r_value + l_value;
  speed = speed / 2000; // porto a scala -1.00 - 1.00

  speed = speed + 0.8; // porto a scala -0.20 - 1.00
  speed = constrain( speed, -0.20, 1.00 );       // annullo eventuali sforamenti
  // gestione retromarcia
  if( speed < 0 ){
    speed = speed * 5; // ho un valore da -0.00 a -0.02 => * 5 porto a scala 0.00 a -1.00
  }

  Serial.print( speed );  
}
void print__turn(){
  float total_turn = 0 - ( r_value - l_value );
  total_turn = total_turn / 2000; // porto a scala -1.00 - 1.00

  int coeff = 6;
  total_turn = total_turn * ( ( abs( total_turn ) * coeff ) + 1 );  // aumento l'efficacia dello sterzo esponenzialmente
  total_turn = map( total_turn, 0 - (coeff + 1), coeff + 1, -1.5, 1.5 ); // https://goo.gl/rXYKvW

  // evito che la sterzatura si presenti anche quando ho le leve circa allineate
  if( total_turn >= -0.01 && total_turn <= 0.01 ){
    total_turn = 0;
  }

  Serial.print( total_turn );
}

