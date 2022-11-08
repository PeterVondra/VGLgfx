Vector3f d = sphere.getPosition() - sphere2.getPosition();
float dist = d.x * d.x + d.y * d.y + d.z * d.z;

if (dist < (sphere.getRadius() + sphere2.getRadius()) * (sphere.getRadius() + sphere2.getRadius()))
	std::cout << "lol" << std::endl;

// Collision detection for sphere x triangle
Vector3f position;
Vector3f velocity = 0;
Vector3f eRad; // ellipsoid radius
Vector3f eVelocity; // Velocity in ellipsoid space
Vector3f eBasePoint; // ellipsoid space

bool collided = false;
double nearestDist = 0;
Vector3f interseresctPoint;

Matrix4f CBM
(
	1 / eRad.x, 0.0f, 0.0f, 0.0f,
	0.0f, 1 / eRad.y, 0.0f, 0.0f,
	0.0f, 0.0f, 1 / eRad.x, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
);

Vector3f eTriangle[3];
eTriangle[0] = { 0,0,0 };
eTriangle[1] = CBM * Vector3f(0, 0, 400);
eTriangle[2] = CBM * Vector3f(400, 0, 0);

Vector3f N = { 0, 1, 0 };

while (!window.closed())
{
	if (Math::dot(N, velocity) > 0)
	{
		double t0, t1;
		bool embeddedInPlane = false;
		double sdfTriangle = Math::dot(N, eBasePoint);

		float NDV = Math::dot(N, velocity);

		if (NDV == 0.0f)
		{
			if (std::fabs(sdfTriangle) >= 1.0f)
				collided = false;
			else
			{
				collided = true;
				embeddedInPlane = true;
				t0 = 0.0f;
				t1 = 1.0f;
			}
		}
		else
		{
			t0 = (-1.0f - sdfTriangle) / NDV;
			t0 = (1.0f - sdfTriangle) / NDV;

			if (t0 > t1)
			{
				double tmp = t1;
				t1 = t0;
				t0 = tmp;
			}
			if (t0 > 1.0f || t1 < 0.0f)
				collided = false;

			if (t0 < 0.0f) t0 = 0.0f;
			if (t0 > 1.0f) t0 = 1.0f;
			if (t1 < 0.0f) t1 = 0.0f;
			if (t1 > 1.0f) t1 = 1.0f;

			//if (!embeddedInPlane)
			//{
			//	Vector3f planeIntersectP = eBasePoint - N + eVelocity * t0;
			//
			//}
		}
	}

	if (collided)
		std::cout << "lol" << std::endl;