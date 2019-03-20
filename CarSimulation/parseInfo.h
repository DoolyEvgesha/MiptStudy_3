#ifndef PARS_H
#define PARS_H

#define PATH_SIZE 20

class Crossings{
private:
    int                     num_;
    //Location            lct[num];
    std::vector <Location>  lct_;
public:
    Crossings               (int num);
    ~Crossings              ();
    bool openDocument       ();
    bool getCrossings       ();
    bool checkCrossing      (Location * location);    //check if there is a crossing on the spot
};

#endif  PARS_H
