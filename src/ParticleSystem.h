#pragma once
//  Kevin M. Smith - CS 134 SJSU

#include "ofMain.h"
#include "Particle.h"


//  Pure Virtual Function Class - must be subclassed to create new forces.
//
class ParticleForce {
protected:
public:
	virtual void updateForce(Particle *) = 0;
};

class ParticleSystem {
public:
	void add(const Particle &);
	void addForce(ParticleForce *);
    int size(){ return particles.size(); }
	void removeForces() { forces.clear(); }
	void remove(int);
    void clear();
	void update();
	void setLifespan(float);
    void setVelocity(ofVec3f vel);
    bool onTallGround(ParticleSystem *);
    bool onLowGround(ParticleSystem *);
	void reset();
	int removeNear(const ofVec3f & point, float dist);
	void draw();
	vector<Particle> particles;
	vector<ParticleForce *> forces;
};



// Some convenient built-in forces
//
class GravityForce: public ParticleForce {
	ofVec3f gravity;
public:
	void setGravity(const ofVec3f &g) { gravity = g; }
	GravityForce(const ofVec3f & gravity);
	GravityForce() {}
	void updateForce(Particle *);
};

class TurbulenceForce : public ParticleForce {
	ofVec3f tmin, tmax;
public:
	void setTubulence(const ofVec3f &min, const ofVec3f &max) { tmin = min; tmax = max; }
	TurbulenceForce(const ofVec3f & min, const ofVec3f &max);
	TurbulenceForce() { tmin.set(0, 0, 0); tmax.set(0, 0, 0); }
	void updateForce(Particle *);
};

class ImpulseRadialForce : public ParticleForce {
	float magnitude = 100.0;
	float height = .2;
    ofVec3f dir = ofVec3f(0,0,0);
public:
	void setImpulseRadialForce(const float mag) { magnitude = mag; }
    void setDir(const ofVec3f direction) { dir = direction; }
	void setHeight(float h) { height = h; }
	ImpulseRadialForce(float magnitude);
	ImpulseRadialForce() {}
	void updateForce(Particle *);
};

class ImpulseForce : public ParticleForce {
    float magnitude = 100.0;
    float height = .2;
    ofVec3f dir = ofVec3f(0,0,0);
public:
    void setImpulseForce(const float mag) { magnitude = mag; }
    void setDir(const ofVec3f direction) { dir = direction; }
    void setHeight(float h) { height = h; }
    ImpulseForce(float magnitude);
    ImpulseForce() {}
    void updateForce(Particle *);
};

class CyclicForce : public ParticleForce {
	float magnitude = 1.0;
public:
	void set(float mag) { magnitude = mag; }
	CyclicForce(float magnitude);  
	CyclicForce() {}
	void updateForce(Particle *);
};

