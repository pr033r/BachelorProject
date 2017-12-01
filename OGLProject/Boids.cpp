
#include "Boids.hpp"

Boids::Boids(float x, float y, float z, Config *cfg)
{
	acceleration = new MyVector();
	velocity = new MyVector((float)(rand() % 3 - 2), (float)(rand() % 3 - 2), (float)(rand() % 3 - 2));
	location = new MyVector(x, y, z);
	size.x = (float)(cfg->BOID_OBJ_SIZE);
	size.y = size.x;
	size.z = size.x;
	this->desiredseparation = cfg->BOID_DESIRED_SEPARATION;
	this->neighbordist = cfg->BOID_NEIGHTBORDIST;
	this->maxSpeed = cfg->BOID_MAX_SPEED;
	this->maxForce = cfg->BOID_MAX_FORCE;
	this->cubeSize = cfg->BOID_CUBE_SIZE / 2;
	this->steer = new MyVector();
	this->sum = new MyVector();
	this->desired = new MyVector();
	this->oppositeVector = new MyVector();
	this->tmpVector = new MyVector();
	this->desiredAvarage = new MyVector();
	this->origin = new MyVector(0, 0, 0);
	this->cfg = cfg;
}

Boids::Boids(Config *cfg) {
	Boids(0.0, 0.0, 0.0, cfg);
}

Boids::~Boids() {
	delete this->oppositeVector;
	delete this->steer;
	delete this->sum;
	delete this->desired;
	delete this->location;
	delete this->velocity;
	delete this->acceleration;
	delete this->tmpVector;
	delete this->desiredAvarage;
	delete this->origin;
}

Boids::Boids(float x, float y, float z, Config *cfg, bool predCheck)
{
	predator = predCheck;
	if (predCheck == true) {
		this->maxSpeed = cfg->BOID_MAX_SPEED_PREDATOR;
		this->maxForce = cfg->BOID_MAX_FORCE_PREDATOR;
		velocity = new MyVector((float)(rand() % 3 - 1), (float)(rand() % 3 - 1), (float)(rand() % 3 - 1));
	}
	else {
		this->maxSpeed = cfg->BOID_MAX_SPEED;
		this->maxForce = cfg->BOID_MAX_FORCE;
		velocity = new MyVector((float)(rand() % 3 - 2), (float)(rand() % 3 - 2), (float)(rand() % 3 - 2));
	}
	acceleration = new MyVector();
	location = new MyVector(x, y, z);
	size.x = (float)(cfg->BOID_OBJ_SIZE);
	this->cubeSize = cfg->BOID_CUBE_SIZE / 2;
	size.y = size.x;
	size.z = size.x;
	this->steer = new MyVector();
	this->sum = new MyVector();
	this->desired = new MyVector();
	this->oppositeVector = new MyVector();
	this->tmpVector = new MyVector();
	this->desiredAvarage = new MyVector();
	this->origin = new MyVector(0, 0, 0);
	this->cfg = cfg;
}

inline void Boids::applyForce(MyVector *force)
{
	acceleration->addVector(force);
}

// Separation
// Keeps Boids from getting too close to one another
MyVector *Boids::Separation(vector<Boids*> *Boidss)
{
	this->steer->set();
	int count = 0;
	
	for (register unsigned int i = 0; i < Boidss->size(); i++) {
		float d = location->distance(Boidss->at(i)->location);
		if ((d > 0) && (d < this->desiredseparation)) {
			this->tmpVector->set();
			this->tmpVectorMem = this->tmpVector;
			this->tmpVector = this->tmpVector->subTwoVector(location, Boidss->at(i)->location);
			this->tmpVector->normalize();
			this->tmpVector->divScalar(d);
			this->steer->addVector(this->tmpVector);
			delete this->tmpVector;
			this->tmpVector = this->tmpVectorMem;
			count++;
		}
		
		if ((d > 0) && (d < this->desiredseparation) && predator == true
			&& Boidss->at(i)->predator == true) {
			this->tmpVector->set();
			this->tmpVectorMem = this->tmpVector;
			this->tmpVector = this->tmpVector->subTwoVector(location, Boidss->at(i)->location);
			this->tmpVector->normalize();
			this->tmpVector->divScalar(d);
			this->steer->addVector(this->tmpVector);
			delete this->tmpVector;
			this->tmpVector = this->tmpVectorMem;
			count++;
		}
		
		else if ((d > 0) && (d < this->desiredseparation) && Boidss->at(i)->predator == true) {
			this->tmpVector->set();
			this->tmpVectorMem = this->tmpVector;
			this->tmpVector = this->tmpVector->subTwoVector(location, Boidss->at(i)->location);
			this->tmpVector->mulScalar(10);
			this->steer->addVector(this->tmpVector);
			delete this->tmpVector;
			this->tmpVector = this->tmpVectorMem;
			count++;
		}
	}
	
	if (count > 0)
		this->steer->divScalar((float)count);
	if (this->steer->magnitude() > 0) {
		// Steering = Desired - Velocity
		this->steer->normalize();
		this->steer->mulScalar(maxSpeed);
		this->steer->subVector(velocity);
		this->steer->limit(maxForce);
	}
	return this->steer;
}

// Alignment
// Calculates the average velocity of Boids in the field of vision and
// manipulates the velocity of the current Boids in order to match it
MyVector *Boids::Alignment(vector<Boids*> *Boidss)
{
	this->sum->set();
	int count = 0;
	for (register unsigned int i = 0; i < Boidss->size(); i++) {
		float d = location->distance(Boidss->at(i)->location);
		if ((d > 0) && (d < this->neighbordist)) {
			this->sum->addVector(Boidss->at(i)->velocity);
			count++;
		}
	}
	
	if (count > 0) {
		this->sum->divScalar((float)count);// Divide sum by the number of close Boids (average of velocity)
		this->sum->normalize(); 
		this->sum->mulScalar(maxSpeed);
									// Steer = Desired - Velocity
		this->desiredAvarage = this->steer->subTwoVector(this->sum, velocity); //sum = desired(average)
		this->steer->set(this->desiredAvarage->vec.x, this->desiredAvarage->vec.y, this->desiredAvarage->vec.z);
		delete this->desiredAvarage;
		this->steer->limit(maxForce);
		return this->steer;
	}
	else {
		return sum;
	}
}

// Cohesion
// Finds the average location of nearby Boids and manipulates the
// steering force to move in that direction.
MyVector *Boids::Cohesion(vector<Boids*> *Boidss)
{
	this->sum->set();
	int count = 0;
	for (register unsigned int i = 0; i < Boidss->size(); i++) {
		float d = location->distance(Boidss->at(i)->location);
		if ((d > 0) && (d < this->neighbordist)) {
			this->sum->addVector(Boidss->at(i)->location);
			count++;
		}
	}
	if (count > 0) {
		this->sum->divScalar((float)count);
		return seek(this->sum);
	}
	else {
		return this->sum;
	}
}

// Limits the maxSpeed, finds necessary steering force and
// normalizes vectors
MyVector *Boids::seek(MyVector *v)
{
	this->desired->set();
	this->desired->subVector(v);
	this->desired->normalize();
	this->desired->mulScalar(maxSpeed);
	acceleration->subTwoVector(desired, velocity);
	acceleration->limit(maxForce); 
	return acceleration;
}

void Boids::update()
{
	acceleration->mulScalar((float)(.4));
	velocity->addVector(acceleration);
	velocity->limit(maxSpeed);
	location->addVector(velocity);
	acceleration->mulScalar(0);
}

void Boids::run(vector <Boids*> *v)
{
	flock(v);
	update();
}

void Boids::flock(vector<Boids*> *v)
{
	this->separationResult = Separation(v);
	this->aligmentResult   = Alignment(v);
	this->cohesionResult   = Cohesion(v);
	this->wallRepelResult  = WallRepel();
	
	this->separationResult->mulScalar(1.5);
	this->aligmentResult->mulScalar(1.0);
	this->cohesionResult->mulScalar(1.0);
	
	applyForce(this->separationResult);
	applyForce(this->aligmentResult);
	applyForce(this->cohesionResult);
	applyForce(this->wallRepelResult);
}

MyVector *Boids::WallRepel() {
	
	this->oppositeVector->set();
	this->oppositeVector->addVector(location);
	this->oppositeVector->mulScalar(-0.5);
	
	if (location->distance(this->origin) > (cfg->BOID_CUBE_SIZE / 2)) {
		return this->oppositeVector;
	}
	else {
		this->oppositeVector->set();
		return this->oppositeVector;
	}

}

float Boids::angle(MyVector *v) const
{
	return (float)(atan2(v->vec.x, -v->vec.y) * 180 / PI);
}

glm::vec3 Boids::rotationVector(MyVector *v) const {
	glm::vec3 rotation;
	float r = sqrt(pow(v->vec.x, 2) + pow(v->vec.y, 2) + pow(v->vec.z, 2));
	float theta = acos(v->vec.z / r);
	float fi = atan(v->vec.y / v->vec.x);
	rotation.x = r;
	rotation.y = theta;
	rotation.z = fi;
	return rotation;
}