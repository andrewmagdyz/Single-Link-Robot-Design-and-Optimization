#include <iostream>
#include <math.h>
#include<vector>
#include<array>
#include<string>
#include<cmath>
#define g 9.81
#define _USE_MATH_DEFINES

using namespace std;

// define mat class
class mat
{
public:
    string name;
    double ys;
    double density;
};

double checkNumber(string text)
{
    double n;
    do
    {
        cout << text;
        if (!(cin >> n))   // If input is not a number
        {
            cin.clear();   // Reset the error flag
            cin.ignore(10000, '\n'); // Discard the bad input
            n = -1;        // Ensure the loop condition (n < 0) stays true
            cout << "\tInvalid input. Please enter a number\n";
            continue;
        }
        if (n <= 0){
            cout << "\tplease enter a positive number\n";
        }
    }
    while (n < 0);
    return n;
}

// make materials vector
vector<mat> materials =
{
    {"Cast Iron",130.0, 7.3*pow(10,-6)},
    {"Copper Nickel",130.0, 8.94*pow(10,-6)},
    {"Brass",200, 8.73*pow(10,-6)},
    {
        "Aluminum", 241, 2.7*pow(10,-6)
    },
    {
        "Steel", 247, 7.58*pow(10,-6)
    },
    {
        "Acrylic", 72, 1.16*pow(10,-6)
    },
    {
        "Copper",70, 8.92*pow(10,-6)
    },
    {
        "Stainless Steel",275, 7.86*pow(10,-6)
    },
    {
        "Tungsten", 941, 19.25*pow(10,-6)
    }
};

// STRESS PART

// define a generic class for all links to have less redundants
class robotLink
{
public:
    // link length
    double length;
    // mass payload
    double payloadMass;
    // angular acceleration
    double acc;
    // constructor
    robotLink(double l, double mp, double a)
    {
        length=l;
        payloadMass=mp;
        acc=a;
    }
protected:
    // moment
    long double moment;
    // mass of the link
    long double ml;
    // stress
    long double s;
    // moment of inertia
    long double I;
public:
    // to get stress
    long double stress()
    {
        return s;
    }
    // to get moment (Torque)
    long double getTorque()
    {
        return moment;
    }
    // mass of the link
    long double mass()
    {
        return ml;
    }
    // calculate the moment
    void calculateMoment()
    {
        moment = ml * g * (length/2) + payloadMass * g * length + ml * pow(length/2, 2) * acc + payloadMass * pow(length,2) * acc;
    }
};

// class for circle shape link inherits from the robotLink class
class Circle :public robotLink
{
public:
    // ridus
    double r;
    //constructor
    Circle(double l, double mp, double a, double ri):robotLink(l, mp, a)
    {
        r = ri;
    }
    // calculating moment of inertia
    void calculateInertia()
    {
        I = (M_PI * pow(r,4))/64;
    }
    // calculating mass of the link
    void calcMassLink(double row)
    {
        ml = (row) *  (M_PI * r * r * length);
    }
    // calculate the stress
    void calcStress()
    {
        s = (moment * r)/(I);

    }
};

// class for rectangle shape link inherits from the robotLink class
class rectangle :public robotLink
{
public:
    // base and height
    double b, h;

    //constructor
    rectangle(double base, double height, double l, double mp, double a):robotLink(l, mp, a)
    {
        b = base;
        h = height;
    }
    // calculating moment of inertia
    void calculateInertia()
    {
        I = (b * pow(h,3))/12;
    }
    // calculating mass of the link
    void calcMassLink(double row)
    {
        ml = (row) *  (b * h * length);
    }
    // calculate the stress
    void calcStress()
    {
        s = (moment * h) / (2 * I);
    }
};

// function to update mass Link, moment, inertia and stress when changing the link dimensions for the rectangular shape
void update(rectangle& link,int material)
{
    link.calcMassLink(materials[material].density);
    link.calculateMoment();
    link.calculateInertia();
    link.calcStress();
}

// function to update mass Link, moment, inertia and stress when changing the link dimensions for the circular shape
void update(Circle& link,int material)
{
    link.calcMassLink(materials[material].density);
    link.calculateMoment();
    link.calculateInertia();
    link.calcStress();
}

// function to  use the rectangular shape link in calculations
rectangle useRectangle(double l, double mp, double a, double b, double h, int material)
{
    // get sigma of the material
    double sy = materials[material].ys;
    // define the rectangular shape link
    rectangle link(b,h,l,mp,a);
    // calculate the sress and the mass of the link at initial dimensions
    update(link,material);
    // loop to increase the link dimension if the stress greater than the sigma of the material
    while(link.stress() > sy)
    {
        link.b *= 1.01;
        link.h *= 1.01;
        update(link,material);

    }
    // loop to decrease the link dimension if the stress less than the sigma of the material
    while(link.stress() <= (sy))
    {
        double oldB = link.b;
        double oldH = link.h;

        link.b *= 0.99;
        link.h *= 0.99;
        update(link,material);
        // check if the last update make it greater than the sigma or not
        if (link.stress() > sy)
        {
            link.b = oldB;
            link.h = oldH;
            update(link,material);
            break;
        }
    }
    return link;
}

Circle useCircle(double l, double mp, double a, double r, int material)
{
    // get sigma of the material
    double sy = materials[material].ys;
    // define the circular shape link
    Circle link(l,mp,a,r);
    // calculate the sress and the mass of the link at initial dimensions
    update(link,material);
    // loop to increase the link dimension if the stress greater than the sigma of the material
    while(link.stress() > sy)
    {
        link.r *= 1.01;
        update(link,material);
    }
    // loop to decrease the link dimension if the stress less than the sigma of the material
    while(link.stress() <= (sy))
    {
        double oldR = link.r;

        link.r *= 0.99;
        update(link,material);

        // check if the last update make it greater than the sigma or not
        if (link.stress() > sy)
        {
            link.r = oldR;
            update(link,material);
            break;
        }
    }
    return link;
}

// function to print materials name
void materialSelect()
{
    system("cls");
    cout << "\t========================================\n\t\tWelcome to Stress Analysis &\n\t\tLink Dimension Optimization\n\t========================================\n";
    for (int i = 0; i < materials.size(); i++)
    {
        cout << "\t" << i+1 << ". " << materials[i].name << endl;
    }
}

// the main function for calculating stress
int stressAnalysis()
{
    // call the materials list
    materialSelect();
    int material;
    do
    {
        cout << "\tChoose material (1-" << materials.size() << "): ";

        // Check if input is a number
        if (!(cin >> material))
        {
            cout << "\tError: Please enter a numeric choice.\n";
            cin.clear();
            cin.ignore(10000, '\n');
            material = -1; // Force loop to continue
            continue;
        }

        material--; // Convert to 0-based index for the vector

        if (material < 0 || material >= materials.size())
        {
            cout << "\tError: Choice out of range.\n";
        }

    }
    while (material < 0 || material >= materials.size());
    //clear the terminal to start the stress analysis
    system("cls");
    cout << "\t========================================\n\t\tWelcome to Stress Analysis &\n\t\tLink Dimension Optimization\n\t========================================\n";
    cout << "\t1. Rectangle shape\n";
    cout << "\t2. Circle shape\n";
    // a flag to exit loop
    bool c = 1;
    // check for user input to choose rectangle or circle
    while (c)
    {
        cout << "\tChoose from 1 to 2: ";
        int x;
        if (!(cin >> x) || (x < 1 || x > 2))
        {
            cout << "\tError: Wrong choice\n";
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }
        // define link length, mass payload and angular acceleration
        double l, mp,a;
        l = checkNumber("\tEnter Length of the Link in (mm): ");
        mp = checkNumber("\tEnter Payload mass in (kg): ");
        a = checkNumber("\tEnter angular acceleration in (rad/s^2): ");
        switch(x)
        {
        // if chosen rectangle
        case 1:
        {
            // make the flag false to exit the loop
            c = 0;
            // define the base and height in milli
            double b, h;
            b = checkNumber("\tEnter base size in (mm): ");
            h = checkNumber("\tEnter Height size in (mm): ");
            // do the analysis
            rectangle Link = useRectangle(l,mp,a,b,h,material);
            // print the result
            cout << "\tDimensions: base = " << Link.b << "mm\theight = " << Link.h << "mm\n";
            cout << "\tMass of the link= " << Link.mass() << "kg\n";
            cout << "\tStress of the link= " << Link.stress() << "Mpa\n";
            cout << "\tYield Stress of Material = " << materials[material].ys << "Mpa\n";
            // return the torque
            return Link.getTorque();
        }
        break;
        // if chosen circle
        case 2:
        {
            // make the flag false to exit the loop
            c = 0;
            // define the redius in milli
            double r;
            r = checkNumber("\tEnter radius in (mm): ");

            // do the analysis
            Circle Link = useCircle(l,mp,a,r, material);

            // print the result
            cout << "\tDimensions: radius = " << Link.r <<  "mm\n";
            cout << "\tMass of the link= " << Link.mass() << "kg\n";
            cout << "\tStress of the link= " << Link.stress() <<"Mpa\n";
            cout << "\tYield Stress of Material = " << materials[material].ys << "Mpa\n";
            // return the torque
            return Link.getTorque();
        }
        break;
        }
    }
}

// MOTOR PART

// motors class
class Motor
{
public:
    string name;
    long double torque;
    long double speed;
    long double mass;
    long double diameter;
    long double width;
};

// gearbox class
class Gearbox
{
public:
    string name;
    long double rati;
    long double efficiency;
    long double mass;
    long double diameter;
    long double width;
};

// my motor database
vector<Motor> motorDatabase=
{
    // name    torque    speed  mass   diameter width
    {"RE 10",  0.00076,  168.6, 0.007,  10.0, 17.0},

    {"EC-max 16",  0.00323,   611.6, 0.036,  16.0, 24.0},

    {"EC-4 pole 30",  0.0956,   1686.0, 0.3,  30.0, 64.0},

    {"RE 65",     0.5010,   399.0, 2.1,  65.0, 131.4},

    {"EC 22",     0.0566,   1194.0, 0.210,  22.0, 90.5},

    {"EC-i 40", 0.0848, 913.2, 0.240, 40.0, 36.0},

    {"RE 50", 0.420, 483.8, 1.1, 50.0, 108.0},

    {"A-max 22", 0.00622, 764.5, 0.054, 22.0, 31.9},

    {"IDX 56 S", 0.475, 592.7, 0.575, 56.0, 107.0},

    {"DCX 6 M", 0.000326, 601.1, 0.002,6.0, 15.6}
};

// my gearbox database
vector<Gearbox> gearboxDatabase=
{
    // name              ratio   eff     mass    diameter width
    {"GP 16 A (4.4:1)",   4.4,   0.90,   0.020,   16.0,   15.5},

    {"GPX 42 UP Ultra (1526:1)", 1526.0, 0.87, 0.790, 42.0, 104.5},

    {"GP 32 HD (14:1)",   14.0,   0.87,   0.234,   32.0,   45.3},

    {"GPX 4 C Ceramic (17:1)",   17.0,   0.76,   0.001,   4.0,   9.25},

    {"GPX 12 (3.9:1)",   3.9,   0.90,   0.011,   12.0,   15.5},

    {"GPX 19 C Ceramic (16:1)",   16.0,   0.80,   0.040,   19.0,   22.9},

    {"GPX 19 LN Low Noise (243:1)", 243.0, 0.90, 0.055, 19.0, 31.0},

    {"GP 22 C Ceramic (16:1)",   16.0,   0.81,   0.058,   22.0,   26.4},

    {"GPX 22 LN Low Noise (243:1)", 243.0, 0.66, 0.089, 22.0, 43.0},

    {"GPX 70 A (44:1)",   44.0,   0.92,   1.342,   70.0,   80.0}
};

//the main function
int motor(long double Tr)
{
    //clear the system
    system("cls");
    // get the req w from the user
    double Wr;
    // set high cost to compare with
    double bestCost = 999999.0;
    Motor bestMotor;
    Gearbox bestGearbox;
    double bestweight=999999999.0;
    cout << "\t========================================\n\t\tWelcome to Motor & GearBox\n\t\t Selection Optimization\n\t========================================\n";
    Wr = checkNumber("\tEnter the required operational speed (rad/s): ");
    int optimization_choice;
    cout<<"\t============================="<<endl;
    cout<<"\tOptimization Priority "<<endl;
    cout<<"\t============================="<<endl;
    cout<<"\t[1] Cost"<<endl;
    cout<<"\t[2] Weight"<<endl;
    do
    {
        cout << "\tChoose 1 or 2: ";

        // 1. Check if the input is NOT a number
        if (!(cin >> optimization_choice))
        {
            cout << "\tError: Please enter a number.\n";
            cin.clear();            // Reset the 'check engine' light
            cin.ignore(10000, '\n'); // Clear the 'trash' from the buffer
            optimization_choice = 0; // Set to a value that keeps the loop going
            continue;
        }

        // 2. Check if the number is within range
        if (optimization_choice < 1 || optimization_choice > 2)
        {
            cout << "\tWrong choice. Please select 1 or 2.\n";
        }

    }
    while (optimization_choice != 1 && optimization_choice != 2);
    // loop for every combination of motor with gearbox
    for (int i = 0; i < motorDatabase.size(); i++)
    {
        for (int j = 0; j < gearboxDatabase.size(); j++)
        {
            // calc the torque and w output
            double t_out = motorDatabase[i].torque * gearboxDatabase[j].rati * gearboxDatabase[j].efficiency;
            double w_out = motorDatabase[i].speed / gearboxDatabase[j].rati;

            // if they are more than required or not
            if (t_out >= Tr && w_out >= Wr)
            {
                // calc mass , width, and the bigger diameter of both
                double totalMass = motorDatabase[i].mass + gearboxDatabase[j].mass;
                double maxWidth = motorDatabase[i].width + gearboxDatabase[j].width;
                double maxDiameter = max(motorDatabase[i].diameter, gearboxDatabase[j].diameter);

                // calc the cost
                double currentCost = totalMass + (maxDiameter/100.0) + (maxWidth/100.0);

                double currentweight=totalMass*9.81;
                switch (optimization_choice)
                {
                case 1:
                    // check if it is the lower cost and save it
                    if (currentCost < bestCost)
                    {
                        bestCost = currentCost;
                        bestMotor = motorDatabase[i];
                        bestGearbox = gearboxDatabase[j];
                        bestweight = currentweight;
                    }
                case 2 :
                    // check if it is the lower weight and save it
                    if(currentweight < bestweight )
                    {
                        bestweight = currentweight;
                        bestMotor = motorDatabase[i];
                        bestGearbox = gearboxDatabase[j];
                        bestCost = currentCost;
                    }
                }
            }
        }
    }
    // print the result
    cout << "\tTorque required= " << Tr << endl;
    if(bestCost == 999999.0 && bestweight == 999999999.0)
    {
        cout << "\tCan't find optimum motor and gearbox\n";
        return 0;
    }
    cout << "\tMotor: " << bestMotor.name;
    cout << "\n\tGearbox: " << bestGearbox.name;
    cout << "\n\tCost= " << bestCost << endl;
    cout << "\tWeight= " << bestweight << endl;
    return 0;
}

int main()
{
    int x = 0;
    do
    {
        system("cls");
        cout << "\t========================================\n"
             << "\t\tWelcome to Final Project\n"
             << "\t========================================\n"
             << "\t1. Start\n"
             << "\t2. Exit\n"
             << "\tChoose 1 or 2: ";

        // 1. Robust Input Check
        if (!(cin >> x))
        {
            cout << "\tInvalid input! Please enter a number.\n\t";
            cin.clear();
            cin.ignore(10000, '\n');
            system("pause");
            continue; // Restart loop to show menu again
        }

        switch (x)
        {
        case 1:
        {
            long double torque = stressAnalysis();
            cout << "\t";
            system("pause");

            motor(torque * 0.001);
            cout << "\t";
            system("pause");
            break;
        }
        case 2:
            cout << "\tBye Bye\n";
            break;
        default:
            cout << "\tError: Choose 1 or 2\n\t";
            system("pause");
            break;
        }
    }
    while (x != 2);
    return 0;
}
