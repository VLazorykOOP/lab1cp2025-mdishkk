#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
using namespace std;

class ErrorRange {
    double rd;
public:
    ErrorRange(double x): rd(x) {}
    void Message() const {
        cout << "ErrorRange: current x is " << rd
             << " but -10 < x < 10 required\n";
    }
    double getValue() const { return rd; }
};

class ErrorNoFile {
    string fname;
public:
    ErrorNoFile(const string& s): fname(s) {}
    void Message() const {
        cout << "ErrorNoFile: cannot open file \"" << fname << "\"\n";
    }
};

class ErrorKrl {
protected:
    double x;
public:
    ErrorKrl(double xx): x(xx) {}
    virtual void Message() const {
        cout << "ErrorKrl: invalid condition at x = " << x << "\n";
    }
    virtual double setFun() const { return x + 1; }
    virtual ~ErrorKrl() = default;
};

class ErrorDKrl : public ErrorKrl {
    double y, z;
public:
    ErrorDKrl(double xx, double yy, double zz)
      : ErrorKrl(xx), y(yy), z(zz) {}
    void Message() const override {
        cout << "ErrorDKrl: invalid at y = " << y
             << " z = " << z << "\n";
    }
    double setFun() const override {
        return int(y + ErrorKrl::setFun());
    }
};

double tbl2(double x) {
    ifstream in("dat1.dat");
    if(!in) throw ErrorNoFile("dat1.dat");
    if(x <= -10 || x >= 10) throw ErrorRange(x);

    double x0, y0, x1, y1;
    in >> x0 >> y0 >> x1 >> y1;
    double prevx = x0, prevy = y0;
    double currx = x1, curry = y1;

    if(fabs(x - prevx) < 1e-9) return prevy;
    if(fabs(x - currx) < 1e-9) return curry;

    while(in >> currx >> curry) {
        if(prevx < x && x < currx) {
            return prevy + (curry - prevy)
                         * ((x - prevx)/(currx - prevx));
        }
        prevx = currx;
        prevy = curry;
    }
    return prevy;
}

double U2(double x) { return atan( asin(sin(3*x)) ); }
double T2(double x) { return atan( acos(sin(2*x)) ); }
double Q2(double x, double y) {
    return x/U2(x) + y*T2(y) - U2(y)*T2(y);
}

double func3(double x, double y, double z) {
    double qxy = Q2(x,y), qyx = Q2(y,x);
    return 1.75*qxy + 1.25*y*qyx - 1.5*qxy*qyx;
}

double Krl2(double x, double y, double z) {
    if(x>0 && x<=1) {
        if(z==0)      throw ErrorKrl(x);
        else          return tbl2(x) + tbl2(y)/z;
    }
    if(y>1) {
        if(x==0)      throw ErrorDKrl(x,y,z);
        else          return tbl2(y) + tbl2(z)/x;
    }
    if(y==0)         throw ErrorKrl(y);
    return tbl2(z) + tbl2(x)/z;
}

double Nrl2(double x, double y) {
    double tmp = sqrt(x*x + y*y);
    if(tmp==0)       throw ErrorDKrl(x,y,0);
    double v = (x>y)
        ? 0.42 * Krl2(x,y,x)
        : 0.57 * Krl2(y,x,y);
    return v - 0.42 * Krl2(x,y,x+y);
}

double Grl2(double x, double y, double z) {
    try {
        if(floor(x+y)==floor(z)) throw double(z);
        if(x+y >= z)
            return 0.3 * Nrl2(x,y) + 0.7 * Nrl2(y,z);
        else
            return 1.3 * Nrl2(x,z) - 0.3 * Nrl2(y,x);
    }
    catch(const ErrorDKrl& e) {
        e.Message(); return e.setFun();
    }
    catch(const ErrorKrl&  e) {
        e.Message(); return e.setFun();
    }
    catch(double d) {
        throw;
    }
}

double fun(double x, double y, double z) {
    return Grl2(x,y, x+y)
         + Grl2(y,z, y+z) * Grl2(x,y, x+y);
}

int main() {
    double x,y,z;
    cout << "Input x y z: ";
    if(!(cin>>x>>y>>z)) {
        cerr<<"Invalid input format\n";
        return 1;
    }

    try {
        double r = fun(x,y,z);
        cout<<"Result = "<< r <<"\n";
    }
    catch(const ErrorRange& e) {
        cout<<"\nCaught ErrorRange\n";  
        e.Message();
        cout<<"Fallback = "<< e.getValue()/10 <<"\n";
    }
    catch(const ErrorNoFile& e) {
        cout<<"\nCaught ErrorNoFile\n"; 
        e.Message();
        cout<<"Fallback = "<< 1.3498*x + 2.2362*y - 2.348*x*y*z <<"\n";
    }
    catch(double d) {
        cout<<"\nCaught double exception: d="<<d<<"\n";
    }
    catch(...) {
        cout<<"\nCaught unknown exception\n";
    }
    return 0;
}
