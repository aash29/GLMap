#ifndef CAR_HPP
#define CAR_HPP

#include "Box2D/Box2D.h"
#include <algorithm>




/*
# Top - down car dynamics simulation.
#
# Some ideas are taken from this great tutorial http ://www.iforce2d.net/b2dtut/top-down-car by Chris Campbell.
# This simulation is a bit more detailed, with wheels rotation.
#
# Created by Oleg Klimov.Licensed on the same terms as the rest of OpenAI Gym.
*/
float CARSIZE = 0.02;
float ENGINE_POWER = 100000000 * CARSIZE*CARSIZE;
float WHEEL_MOMENT_OF_INERTIA = 4000 * CARSIZE*CARSIZE;
float FRICTION_LIMIT = 1000000 * CARSIZE*CARSIZE;     // friction ~= mass ~= CARSIZE ^ 2 (calculated implicitly using density)
float WHEEL_R = 27;
float WHEEL_W = 14;
float WHEELPOS[4][2] = {
	{-45, +80}, {+45, +80}, {-45, -82}, {+45, -82}
};

b2Vec2 HULL_POLY1[4] = {
	CARSIZE*b2Vec2 (-60, +130), CARSIZE*b2Vec2 (+60, +130),
	CARSIZE*b2Vec2 (+60, +110), CARSIZE*b2Vec2(-60, +110)
};

b2Vec2 HULL_POLY2[4] = {
	CARSIZE*b2Vec2(-35, +120), CARSIZE*b2Vec2 (+35, +120),
	CARSIZE*b2Vec2(+40, +10), CARSIZE*b2Vec2 (-40, 10)
};

b2Vec2 HULL_POLY3[8] = {
	CARSIZE*b2Vec2(+25, +20),
	CARSIZE*b2Vec2(+50, -10),
	CARSIZE*b2Vec2(+50, -40),
	CARSIZE*b2Vec2(+20, -90),
	CARSIZE*b2Vec2(-20, -90),
	CARSIZE*b2Vec2(-50, -40),
	CARSIZE*b2Vec2(-50, -10),
	CARSIZE*b2Vec2(-25, +20)
};

b2Vec2 HULL_POLY4[4] = {
	CARSIZE*b2Vec2(-50, -120), CARSIZE*b2Vec2(+50, -120),
	CARSIZE*b2Vec2(+50, -90), CARSIZE*b2Vec2(-50, -90)
};

float WHEEL_COLOR[3] = { 0.0, 0.0, 0.0 };
float WHEEL_WHITE[3] = { 0.3, 0.3, 0.3 };
float MUD_COLOR[3] = { 0.4, 0.4, 0.0 };

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

struct wheel {
	
	float wheel_rad;
		//w.color = WHEEL_COLOR
	float gas;

	float brake;
	float steer;
	float phase; // wheel angle
	float omega; // angular velocity
	b2Body* body;
	b2RevoluteJoint* joint;
	//float skid_start = None
	//	w.skid_particle = None
};

class Car
{

	
public:

	b2World *world;
	b2Body *hull;
	wheel wheels[4];

	Car(b2World *world, float init_angle, float init_x, float init_y) {
		this->world = world;
		b2BodyDef hulldef;

		hulldef.position = b2Vec2(init_x, init_y);
		hulldef.angle = init_angle;
		hulldef.type = b2_dynamicBody;
		this->hull = this->world->CreateBody(&hulldef);

		b2FixtureDef fd1;
		b2PolygonShape hp1;
		hp1.Set(HULL_POLY1, 4);
		fd1.shape = &hp1;
		fd1.density = 1.f;
		this->hull->CreateFixture(&fd1);

		b2FixtureDef fd2;
		b2PolygonShape hp2;
		hp2.Set(HULL_POLY2, 4);
		fd2.shape = &hp2;
		fd2.density = 1.f;
		this->hull->CreateFixture(&fd2);

		b2FixtureDef fd3;
		b2PolygonShape hp3;
		hp3.Set(HULL_POLY3, 8);
		fd3.shape = &hp3;
		fd3.density = 1.f;
		this->hull->CreateFixture(&fd3);

		b2FixtureDef fd4;
		b2PolygonShape hp4;
		hp4.Set(HULL_POLY4, 4);
		fd4.shape = &hp4;
		fd4.density = 1.f;
		this->hull->CreateFixture(&fd4);
	
		b2Vec2 WHEEL_POLY[4] = {
			CARSIZE*b2Vec2(-WHEEL_W, +WHEEL_R), CARSIZE*b2Vec2(+WHEEL_W, +WHEEL_R),
			CARSIZE*b2Vec2(+WHEEL_W, -WHEEL_R), CARSIZE*b2Vec2(-WHEEL_W, -WHEEL_R)
		};
		int iw = 0;
		for (float* wv1 : WHEELPOS) {
			float front_k = 1.0; // if wy > 0 else 1.0

			//b2Vec2 position(init_x + wv1[0]*CARSIZE, init_y + wv1[1]*CARSIZE);
			b2BodyDef wheeldef;


			wheeldef.position = b2Vec2(init_x + wv1[0] * CARSIZE, init_y + wv1[1] * CARSIZE);
			wheeldef.angle = init_angle;
			wheeldef.type = b2_dynamicBody;


			b2PolygonShape wsh1;
			wsh1.Set(WHEEL_POLY, 4);
			b2FixtureDef wfd1;
			wfd1.shape = &wsh1;
			wfd1.density = 0.1;
			wfd1.restitution = 0.f;
			wfd1.filter.categoryBits = 0x0020;
			wfd1.filter.maskBits = 0x001;
			

			b2Body* w = this->world->CreateBody(&wheeldef);

			w->CreateFixture(&wfd1);
			

			b2RevoluteJointDef rjdef;
			rjdef.bodyA = this->hull;
			rjdef.bodyB = w;
			rjdef.localAnchorA = b2Vec2(wv1[0] * CARSIZE, wv1[1] * CARSIZE);
			rjdef.localAnchorB = b2Vec2(0, 0);
			rjdef.enableMotor = true;
			rjdef.enableLimit = true;
			rjdef.maxMotorTorque = 180 * 900 * CARSIZE*CARSIZE;
			rjdef.motorSpeed = 0;
			rjdef.lowerAngle = -0.4;
			rjdef.upperAngle = +0.4;
			b2RevoluteJoint* rjd = (b2RevoluteJoint*)this->world->CreateJoint(&rjdef);
			wheels[iw] = { WHEEL_R*CARSIZE, 0.f, 0.f, 0.f ,0.f, 0.f, w, rjd };
			iw++;
		}
	}
	void accelerate(float gas) {
		gas = (std::max)(-1.f,(std::min)(gas, 1.f));
		
		for (int i = 2; i <= 3; i ++) {
			float diff = gas - wheels[i].gas;
			if (diff > 0.1) {
				diff = 0.1;
			};  // gradually increase, but stop immediately
			wheels[i].gas += diff;
		};
	}


	void brake(float b) { //	'control: brake b=0..1, more than 0.9 blocks wheels to zero rotation'
		for (int i = 0; i < 4; i++) {
				wheels[i].brake = b;
			}
		
	}
	void steer(float s) { //'control: steer s=-1..1, it takes time to rotate steering wheel from side to side, s is target position'
		wheels[0].steer = s;
		wheels[1].steer = s;
	}

	void step(float dt) {
		for (auto &w : wheels) {
			//# Steer each wheel
			float dir = sgn(w.steer - w.joint->GetJointAngle());
			float val = abs(w.steer - w.joint->GetJointAngle());
			w.joint->SetMotorSpeed(dir*min(50.0*val, 3.0));

			//# Position = > friction_limit
			//grass = True
			float friction_limit = FRICTION_LIMIT*0.8;  //# Grass friction if no tile
														   //for tile in w.tiles:
														   //float friction_limit = max(friction_limit, FRICTION_LIMIT*tile.road_friction)
														   //grass = False


			b2Vec2 forw = w.body->GetWorldVector(b2Vec2(0.f, 1.f));
			b2Vec2 side = w.body->GetWorldVector(b2Vec2(1.f, 0.f));
			b2Vec2 v = w.body->GetLinearVelocity();
			float vf = forw(0) * v(0) + forw(1) * v(1);  //# forward speed
			float vs = side(0) * v(0) + side(1) * v(1);  //# side speed

				//# WHEEL_MOMENT_OF_INERTIA*np.square(w.omega) / 2 = E -- energy
				//# WHEEL_MOMENT_OF_INERTIA*w.omega * domega / dt = dE / dt = W -- power
				//# domega = dt*W / WHEEL_MOMENT_OF_INERTIA / w.omega
			w.omega += dt*ENGINE_POWER*w.gas / WHEEL_MOMENT_OF_INERTIA / (abs(w.omega) + 5.0);  //# small coef not to divide by zero
				///self.fuel_spent += dt*ENGINE_POWER*w.gas

			if (w.brake >= 0.9) {
				w.omega = 0;
			}
			else if (w.brake > 0) {
				float BRAKE_FORCE = 15;   //    # radians per second
				float dir = -sgn(w.omega);
				float val = BRAKE_FORCE*w.brake;
				if (abs(val) > abs(w.omega)) {
					val = abs(w.omega);  //# low speed = > same as = 0
				}
				w.omega += dir*val;
			}

			w.phase += w.omega*dt;

			float vr = w.omega*w.wheel_rad;  //# rotating wheel speed
			float f_force = -vf + vr;        //# force direction is direction of speed difference
			float p_force = -vs;

			//# Physically correct is to always apply friction_limit until speed is equal.
			//# But dt is finite, that will lead to oscillations if difference is already near zero.
			f_force = f_force * 205000 * CARSIZE*CARSIZE;  //# Random coefficient to cut oscillations in few steps(have no effect on friction_limit)
			p_force = p_force * 205000 * CARSIZE*CARSIZE;
			float force = sqrt(f_force*f_force + p_force*p_force);

			if (abs(force) > friction_limit) {
				f_force /= force;
				p_force /= force;
				force = friction_limit;  //# Correct physics here
				f_force *= force;
				p_force *= force;
			}
			w.omega -= dt*f_force*w.wheel_rad / WHEEL_MOMENT_OF_INERTIA;

			w.body->ApplyForceToCenter(b2Vec2(
				p_force*side(0) + f_force*forw(0),
				p_force*side(1) + f_force*forw(1)), true);

		}
	}
};
			


							/*
	def gas(self, gas) :
							'control: rear wheel drive'
							gas = np.clip(gas, 0, 1)
							for w in self.wheels[2:4] :
								diff = gas - w.gas
								if diff > 0.1: diff = 0.1  # gradually increase, but stop immediately
									w.gas += diff

					def brake(self, b) :
									'control: brake b=0..1, more than 0.9 blocks wheels to zero rotation'
									for w in self.wheels :
										w.brake = b

										def steer(self, s) :
										'control: steer s=-1..1, it takes time to rotate steering wheel from side to side, s is target position'
										self.wheels[0].steer = s
										self.wheels[1].steer = s

										def step(self, dt) :
										for w in self.wheels :
											# Steer each wheel
											dir = np.sign(w.steer - w.joint.angle)
											val = abs(w.steer - w.joint.angle)
											w.joint.motorSpeed = dir*min(50.0*val, 3.0)

											# Position = > friction_limit
											grass = True
											friction_limit = FRICTION_LIMIT*0.6  # Grass friction if no tile
											for tile in w.tiles:
						friction_limit = max(friction_limit, FRICTION_LIMIT*tile.road_friction)
							grass = False

							# Force
							forw = w.GetWorldVector((0, 1))
							side = w.GetWorldVector((1, 0))
							v = w.linearVelocity
							vf = forw[0] * v[0] + forw[1] * v[1]  # forward speed
							vs = side[0] * v[0] + side[1] * v[1]  # side speed

							# WHEEL_MOMENT_OF_INERTIA*np.square(w.omega) / 2 = E -- energy
							# WHEEL_MOMENT_OF_INERTIA*w.omega * domega / dt = dE / dt = W -- power
							# domega = dt*W / WHEEL_MOMENT_OF_INERTIA / w.omega
							w.omega += dt*ENGINE_POWER*w.gas / WHEEL_MOMENT_OF_INERTIA / (abs(w.omega) + 5.0)  # small coef not to divide by zero
							self.fuel_spent += dt*ENGINE_POWER*w.gas

							if w.brake >= 0.9:
						w.omega = 0
							elif w.brake > 0:
						BRAKE_FORCE = 15    # radians per second
							dir = -np.sign(w.omega)
							val = BRAKE_FORCE*w.brake
							if abs(val) > abs(w.omega) : val = abs(w.omega)  # low speed = > same as = 0
								w.omega += dir*val
								w.phase += w.omega*dt

								vr = w.omega*w.wheel_rad  # rotating wheel speed
								f_force = -vf + vr        # force direction is direction of speed difference
								p_force = -vs

								# Physically correct is to always apply friction_limit until speed is equal.
								# But dt is finite, that will lead to oscillations if difference is already near zero.
								f_force *= 205000 * CARSIZE*CARSIZE  # Random coefficient to cut oscillations in few steps(have no effect on friction_limit)
								p_force *= 205000 * CARSIZE*CARSIZE
								force = np.sqrt(np.square(f_force) + np.square(p_force))

								# Skid trace
								if abs(force) > 2.0*friction_limit:
						if w.skid_particle and w.skid_particle.grass == grass and len(w.skid_particle.poly) < 30 :
							w.skid_particle.poly.append((w.position[0], w.position[1]))
							elif w.skid_start is None :
						w.skid_start = w.position
						else :
							w.skid_particle = self._create_particle(w.skid_start, w.position, grass)
							w.skid_start = None
								else:
						w.skid_start = None
							w.skid_particle = None

							if abs(force) > friction_limit:
						f_force /= force
							p_force /= force
							force = friction_limit  # Correct physics here
							f_force *= force
							p_force *= force

							w.omega -= dt*f_force*w.wheel_rad / WHEEL_MOMENT_OF_INERTIA

							w.ApplyForceToCenter((
								p_force*side[0] + f_force*forw[0],
								p_force*side[1] + f_force*forw[1]), True)

*/
#endif