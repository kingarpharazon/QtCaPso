#ifndef SWARM_H
#define SWARM_H

#include <list>
#include <random>
#include <memory>
#include <QtGlobal>
#include "particle.h"

class Swarm
{
public:
	Swarm(float cognitiveFactor, float socialFactor, int maxSpeed);

	~Swarm();

	float cognitiveFactor() const { return mCognitiveFactor; }
	float socialFactor() const { return mSocialFactor; }
	int maxSpeed() const { return mMaxSpeed; }
	float inertiaWeight() const { return mInertiaWeight; }

	void setCognitiveFactor(float factor) { mCognitiveFactor = factor; }
	void setSocialFactor(float factor) { mSocialFactor = factor; }
	void setMaxSpeed(int speed) { mMaxSpeed = speed; }

	std::list<std::shared_ptr<Particle>>::iterator begin();
	std::list<std::shared_ptr<Particle>>::iterator end();

	void initialize(int size, int width, int height, std::mt19937& random);
	void add(std::list<std::shared_ptr<Particle>>& newParticles);

#if defined(Q_OS_LINUX)
	std::list<std::shared_ptr<Particle>, std::allocator<std::shared_ptr<Particle>>>::iterator erase(std::list<std::shared_ptr<Particle>,
        std::allocator<std::shared_ptr<Particle>>>::iterator it);
#else
    std::list<std::shared_ptr<Particle>, std::allocator<std::shared_ptr<Particle>>>::iterator erase(std::list<std::shared_ptr<Particle>,
        std::allocator<std::shared_ptr<Particle>>>::const_iterator it);
#endif

	bool empty() const;

private:
	std::list<std::shared_ptr<Particle>> mParticles;
	float mCognitiveFactor;
	float mSocialFactor;
	int mMaxSpeed;
	float mInertiaWeight;
};

#endif // SWARM_H
