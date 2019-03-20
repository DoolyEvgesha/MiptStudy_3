#include <iostream>
#include <vector>

#include "parseInfo.h"

Crossings::Crossings(int num):
    num_(num)
{
    try
    {
        lct_.reserve(num_);
    }
    catch(const std::bad_alloc & e)
    {
        std::cout << "Ops, smth wrong with new memory for Location" << std::endl;
    }
}

Crossings::~Crossings()
{
    if(!lct_.empty())
        lct_.clear();
}

bool Crossings::openDocument()
{
    std::fsteam fs;

    char path[PATH_SIZE] = {0};
    std::cout << "Please, enter the name of the file to open:" << std::endl;
    std::cin  >> path >> std::endl;

    fs.open(path, std::fsteam::in);
    Cross = getCrossings(fs);
    fs.close();

    return true;
}

bool Crossings::getCrossings(std::fsteam & fs)
{

}

bool Crossings::checkCrossing(Location *location)
{
    if((location->x_ == lct_.x) && (location->x_ == lct_.x))
        return true;
    return false;
}