#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
using namespace std;

const double G = 6.674e-11;
const double SOFTENING = 1e3;

struct Vec3 {
    double x = 0, y = 0, z = 0;

    Vec3() {}
    Vec3(double X, double Y, double Z) : x(X), y(Y), z(Z) {}

    Vec3 operator+(const Vec3& o) const { return Vec3(x+o.x, y+o.y, z+o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x-o.x, y-o.y, z-o.z); }
    Vec3 operator*(double s) const { return Vec3(x*s, y*s, z*s); }
    Vec3& operator+=(const Vec3& o) { x+=o.x; y+=o.y; z+=o.z; return *this; }

    double normSquared() const { return x*x + y*y + z*z; }
    void reset() { x = y = z = 0; }
};


struct Particle {
    double mass;
    Vec3 pos;
    Vec3 vel;
    Vec3 force;

    Particle(double m, Vec3 p, Vec3 v)
        : mass(m), pos(p), vel(v), force() {}
};


vector<Particle> load_particles_from_file(const string& filename) {
    vector<Particle> particles;
    ifstream in(filename);
    string line;
    getline(in, line); // One line only
    stringstream ss(line);

    int N;
    ss >> N;

    for (int i = 0; i < N; ++i) {
        double m, x, y, z, vx, vy, vz, fx, fy, fz;
        ss >> m >> x >> y >> z >> vx >> vy >> vz >> fx >> fy >> fz;
        particles.emplace_back(m, Vec3(x, y, z), Vec3(vx, vy, vz));
    }

    return particles;
}


void compute_forces(vector<Particle>& particles) {
    for (auto& p : particles) p.force.reset();

    for (size_t i = 0; i < particles.size(); ++i) {
        for (size_t j = i + 1; j < particles.size(); ++j) {
            Vec3 delta = particles[j].pos - particles[i].pos;
            double distSqr = delta.normSquared() + SOFTENING * SOFTENING;
            double dist = sqrt(distSqr);
            double forceMag = G * particles[i].mass * particles[j].mass / distSqr;
            Vec3 forceVec = delta * (forceMag / dist);

            particles[i].force += forceVec;
            particles[j].force += forceVec * -1;
        }
    }
}


void update_particles(vector<Particle>& particles, double dt) {
    for (auto& p : particles) {
        Vec3 accel = p.force * (1.0 / p.mass);
        p.vel += accel * dt;
        p.pos += p.vel * dt;
    }
}


void output_state(ofstream& out, const vector<Particle>& particles) {
    out << particles.size();
    for (const auto& p : particles) {
        out << '\t' << p.mass
            << '\t' << p.pos.x << '\t' << p.pos.y << '\t' << p.pos.z
            << '\t' << p.vel.x << '\t' << p.vel.y << '\t' << p.vel.z
            << '\t' << p.force.x << '\t' << p.force.y << '\t' << p.force.z;
    }
    out << '\n';
}


int main(int argc, char** argv) {
    if (argc < 5) {
        cerr << "Usage: " << argv[0] << " <input_file> <dt> <steps> <dump_freq>\n";
        return 1;
    }

    string input_file = argv[1];
    double dt = atof(argv[2]);
    int steps = atoi(argv[3]);
    int dump_freq = atoi(argv[4]);

    auto particles = load_particles_from_file(input_file);
    ofstream out("output.tsv");

    for (int step = 0; step <= steps; ++step) {
        compute_forces(particles);
        if (step % dump_freq == 0)
            output_state(out, particles);
        update_particles(particles, dt);
    }

    out.close();
    return 0;
}
