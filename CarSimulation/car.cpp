#include <iostream>

#include "car.h"

Car::Car(carId id, Location lct, carParam length, carParam width):
    id_     (id),
    length_ (length),
    width_  (width) //how to initialize Location?
{
    lct_.x_ = length.x;
    lct_.y_ = length.y;
}

bool Car::sendCarCommand(char * command) {command_ = command;}

bool Car::setSpeed(int speed) { speed_ = speed; }
