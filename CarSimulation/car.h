#ifndef CAR_H
#define CAR_H

typedef int carId;     //type of data for car id
typedef int coordType; //type of data for coordinates of car
typedef int carParam;  //type of data of car parameters (lenght, width etc)

enum COMMANDS {
    GO_FORWARD      = 1,
    TURN_LEFT       = 2,
    TURN_RIGHT      = 3,
    STOP            = 4,
    FINISH          = 5
};


char * MovMessages[] = {
        "Turn LEFT",
        "Turn RIGHT",
        "Go FORWARD",
        "Stop",
        "Finish" // equivalent to end
};

char * Actions[] = {
        "turn_left",
        "turn_right",
        "go_forward",
        "stop",
        "end"
};

// Variable "Location" can be used both for
// the locations of the cars and
// the crossing on the road
// (положение машин и перекрестков)

typedef struct Location {
    coordType   x_;
    coordType   y_;
}Location;

typedef struct CarLocation{
    Location preLoc; //location of the previous crossing
    Location nextLoc; //location of the next crossing
    Location currentLoc; //location of the current crossing
}CarLocation;

class Car {
private:
    carId       id_;
    Location    lct_;//location of the car
    CarLocation crossloc_; //location of the three crossings:prev, next, current
    carParam    length_;
    carParam    width_;
    int         speed_;
    int         command_;

public:
    Car                         (carId id, Location lct, carParam length, carParam width);
    ~Car                        ();
    bool        sendCarCommand  (char * command);
    bool        setSpeed        (int speed);
    bool        carCalculations ();
};

#endif