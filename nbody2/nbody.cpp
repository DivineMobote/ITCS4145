
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <random>
#include <algorithm>
using namespace std;


// ---------------------------- 3D vector helpers ---------------------------- 
struct Vec3 {
    double x=0, y=0, z=0;
    Vec3()=default;
    Vec3(double X,double Y,double Z):x(X),y(Y),z(Z){}
    Vec3& operator+=(const Vec3& o){ x+=o.x; y+=o.y; z+=o.z; return *this; }
    Vec3& operator-=(const Vec3& o){ x-=o.x; y-=o.y; z-=o.z; return *this; }
    Vec3& operator*=(double s){ x*=s; y*=s; z*=s; return *this; }
};

static inline Vec3 operator+(Vec3 a,const Vec3& b){ a+=b; return a; }
static inline Vec3 operator-(Vec3 a,const Vec3& b){ a-=b; return a; }
static inline Vec3 operator*(Vec3 a,double s){ a*=s; return a; }
static inline Vec3 operator*(double s,Vec3 a){ a*=s; return a; }


// --------------------------- Particle state ---------------------------- 
// Each particle tracks mass, position, velocity and the net force
struct Particle {
    double m;
    Vec3 x, v, f; 
};


// ---------------------------- The core of the simulation ---------------------------- 
// Parameters
struct Sim {
    
    double G = 6.674e-11;
    double soft2 = 1e-9;       
    double dt = 1.0;
    int dump_every = 1;
    unsigned rng_seed = 42;

    // All particles in the system
    vector<Particle> p;


    // Initial modes
    void random_init(size_t n, double mass_min=1e20, double mass_max=1e26,
                     double pos_span=1e11, double vel_span=1e3) {
        p.assign(n, {});
        std::mt19937_64 rng(rng_seed);
        std::uniform_real_distribution<double> Um( mass_min, mass_max );
        std::uniform_real_distribution<double> Up(-pos_span, pos_span);
        std::uniform_real_distribution<double> Uv(-vel_span, vel_span);
        for (auto& q : p) {
            q.m = Um(rng);
            q.x = Vec3(Up(rng), Up(rng), Up(rng));
            q.v = Vec3(Uv(rng), Uv(rng), Uv(rng));
            q.f = Vec3(0,0,0);
        }
    }

    
    // Load initial state format from a TSV line 
    void load_from_tsv(const string& path) {
    p.clear();
    ifstream in(path);
    if (!in) { cerr << "ERROR: cannot open " << path << "\n"; exit(1); }

    size_t n;
    if (!(in >> n)) { cerr << "ERROR: bad header in " << path << "\n"; exit(1); }

    p.resize(n);
    for (size_t i = 0; i < n; ++i) {
        Particle q;
        if (!(in >> q.m
                 >> q.x.x >> q.x.y >> q.x.z
                 >> q.v.x >> q.v.y >> q.v.z
                 >> q.f.x >> q.f.y >> q.f.z)) {
            cerr << "ERROR: not enough fields for particle " << i << "\n"; exit(1);
        }
        p[i] = q;
    }
}


    // 3-body examples in SI unit
    void preset_sun_earth_moon() {
        p.clear(); p.resize(3);
        
        // Sun 
        p[0].m = 1.9891e30;
        p[0].x = Vec3(0,0,0);
        p[0].v = Vec3(0,0,0);
        
        // Earth
        p[1].m = 5.972e24;
        p[1].x = Vec3(1.496e11, 0, 0);
        p[1].v = Vec3(0, 29780, 0);
        
        // Moon
        p[2].m = 7.342e22;
        p[2].x = Vec3(1.496e11 + 3.844e8, 0, 0);
        p[2].v = Vec3(0, 29780 + 1022, 0);
        for(auto& q:p) q.f = Vec3(0,0,0);
    }

    // Clear forces before recomputing each step
    void reset_forces(){ for(auto& q:p) q.f = Vec3(0,0,0); }


    // Compute all gravitational forces 
    void compute_forces() {
        reset_forces();
        const size_t n = p.size();
        for(size_t i=0;i<n;i++){
            for(size_t j=i+1;j<n;j++){
                Vec3 r = p[j].x - p[i].x;
                double r2 = r.x*r.x + r.y*r.y + r.z*r.z + soft2;
                double inv_r = 1.0 / sqrt(r2);
                double inv_r3 = inv_r * inv_r * inv_r;
                double coef = G * p[i].m * p[j].m * inv_r3;
                Vec3 F = coef * r;   
                p[i].f += F;
                p[j].f -= F;         
            }
        }
    }


    // Euler
    void integrate_step() {
        
        for (auto& q : p) {
            Vec3 a = (1.0/q.m) * q.f;
            q.v += a * dt;
            q.x += q.v * dt;
        }
    }


    // Outputs
    static void print_state_line(ostream& os, const vector<Particle>& p) {
        os << p.size();
        os.setf(std::ios::scientific);
        os << setprecision(12);
        for (const auto& q: p) {
            os << '\t' << q.m
               << '\t' << q.x.x << '\t' << q.x.y << '\t' << q.x.z
               << '\t' << q.v.x << '\t' << q.v.y << '\t' << q.v.z
               << '\t' << q.f.x << '\t' << q.f.y << '\t' << q.f.z;
        }
        os << '\n';
    }
};


// Helper: detect if a string looks like a number
static bool is_number(const string& s){
    
    size_t i=0; if(i<s.size() && (s[i]=='+'||s[i]=='-')) i++;
    return i<s.size() && isdigit(static_cast<unsigned char>(s[i]));
}


// ---------------------------- Program entry ---------------------------- 
int main(int argc, char** argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Help with text when args are missing
    if(argc < 5){
        cerr <<
"Usage:\n"
"  ./nbody <N | input.tsv | preset> <dt> <steps> <dump_every> [--out FILE]\n"
"         [--soft EPS] [--G VAL] [--seed S]\n"
"  preset options: sun-earth-moon\n"
"Notes:\n"
"  - If first arg is a number -> random init with N particles.\n"
"  - If 'sun-earth-moon'       -> 3-body preset.\n"
"  - Else                      -> treated as TSV to load first line.\n";
        return 1;
    }


    // Parse the 4 args
    string mode = argv[1];
    Sim sim;
    sim.dt = stod(argv[2]);
    long long steps = atoll(argv[3]);
    sim.dump_every = stoi(argv[4]);

    string out_path = "output.tsv";
    

    // Parse flags
    for(int i=5;i<argc;i++){
        string a = argv[i];
        auto need = [&](const string& flag){
            if(i+1>=argc){ cerr<<"Missing value after "<<flag<<"\n"; exit(1); }
            return string(argv[++i]);
        };
        if(a=="--out") out_path = need("--out");
        else if(a=="--soft") sim.soft2 = stod(need("--soft"));
        else if(a=="--G") sim.G = stod(need("--G"));
        else if(a=="--seed") sim.rng_seed = (unsigned)stoull(need("--seed"));
        else {
            cerr<<"Unknown flag: "<<a<<"\n";
            return 1;
        }
    }


    // Chooses initial mode
    if (mode=="sun-earth-moon") {
        sim.preset_sun_earth_moon();
    } else if (is_number(mode)) {
        size_t N = stoull(mode);
        sim.random_init(N);
    } else {
        sim.load_from_tsv(mode);
    }

    // Open the output file
    ofstream out(out_path);
    if(!out){ cerr<<"ERROR: cannot open output "<<out_path<<"\n"; return 1; }

    // Dumps the initial states (forces currently zero unless loaded from file)
    if(0 % sim.dump_every == 0) Sim::print_state_line(out, sim.p);


    // Main simulation loop
    for(long long t=1; t<=steps; ++t){
        sim.compute_forces();
        sim.integrate_step();
        if(t % sim.dump_every == 0) {
            
            Sim::print_state_line(out, sim.p);
        }
    }

    cerr<<"Wrote states to "<<out_path<<"\n";
    return 0;
}
