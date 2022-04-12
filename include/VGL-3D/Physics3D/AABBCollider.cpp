float getTriArea(Vector3f p1, Vector3f p2, Vector3f p3)
{
	Vector3f l1 = p1 - p2;
	Vector3f l2 = p1 - p3;
	Vector3f l3 = p2 - p3;

	float l1llength = std::sqrtf(Math::dot(l1, l1));
	float l2llength = std::sqrtf(Math::dot(l2, l2));
	float l3llength = std::sqrtf(Math::dot(l3, l3));

	float s = (l1llength + l2llength + l3llength) / 2.0f;
	float area = std::sqrtf(s * (s - l1llength) * (s - l2llength) * (s - l3llength));

	return area;
}

bool intersecting(vgl::Model& model, vgl::Model& test)
{
	bool intersect = false;
	for (int i = 0; i < model.getIndices().size(); i += 3)
	{
		Vector3f tp1 = model.model * model.getVertices()[model.getIndices()[i]].position;
		Vector3f tp2 = model.model * model.getVertices()[model.getIndices()[i + 1]].position;
		Vector3f tp3 = model.model * model.getVertices()[model.getIndices()[i + 2]].position;

		Vector3f triPoint1 = (tp1 + tp2 + tp3) / 3;

		Vector3f nrm1 = Math::normalize(Math::cross(tp2 - tp1, tp3 - tp1));

		

		float area = getTriArea(tp1, tp2, tp3);
		for (int j = 0; j < test.getIndices().size(); j += 3)
		{
			Vector3f p1 = test.model * test.getVertices()[test.getIndices()[j]].position;
			Vector3f p2 = test.model * test.getVertices()[test.getIndices()[j + 1]].position;
			Vector3f p3 = test.model * test.getVertices()[test.getIndices()[j + 2]].position;

			// p1 and p2
			// p1 and p3
			// p2 and p3

			float t1 = -1;
			float t2 = -1;
			float t3 = -1;

			if (p1 != p2)
				t1 = -(Math::dot(nrm1, p2) + D1) / Math::dot(nrm1, p1 - p2);

			if (p1 != p3)
				t2 = -(Math::dot(nrm1, p2) + D1) / Math::dot(nrm1, p1 - p3);

			if (p2 != p3)
				t3 = -(Math::dot(nrm1, p3) + D1) / Math::dot(nrm1, p2 - p3);

			Vector3f int1 = p1 * t1 + p2 * (1 - t1);
			Vector3f int2 = p1 * t2 + p3 * (1 - t2);
			Vector3f int3 = p2 * t3 + p3 * (1 - t3);

			if ((t1 <= 1 && t1 >= 0))
			{
				float area1 = getTriArea(int1, tp1, tp2) + getTriArea(int1, tp1, tp3) + getTriArea(int1, tp2, tp3);

				if (area >= (area1 - 0.01f))
					intersect = true;
			}
			if ((t2 <= 1 && t2 >= 0))
			{
				float area1 = getTriArea(int2, tp1, tp2) + getTriArea(int2, tp1, tp3) + getTriArea(int2, tp2, tp3);

				if (area >= (area1 - 0.01f))
					intersect = true;
			}
			if ((t3 <= 1 && t3 >= 0))
			{
				float area1 = getTriArea(int3, tp1, tp2) + getTriArea(int3, tp1, tp3) + getTriArea(int3, tp2, tp3);

				if (area >= (area1 - 0.01f))
					intersect = true;
			}
		}
	}

	return intersect;
}

bool getMinRoot(float a, float b, float c, float max, float& root)
{
	if (b * b - 4 * a * c < 0)
		return false;

	float sq = std::sqrtf(b * b - 4 * a * c);
	float x1 = (-b - sq) / (2 * a);
	float x2 = (-b + sq) / (2 * a);

	if (x1 > x2)
		std::swap(x1, x2);

	if (x1 > 0 && x1 < max)
	{
		root = x1;
		return true;
	}
	if (x2 > 0 && x2 < max)
	{
		root = x2;
		return true;
	}

	return false;
}

#define in(a) ((uint32_t&) a) 
bool checkPointInTriangle(const Vector3f& point, const Vector3f& pa, const Vector3f& pb, const Vector3f& pc) {
	Vector3f e10 = pb - pa;
	Vector3f e20 = pc - pa;
	float a = Math::dot(e10, e10);
	float b = Math::dot(e10, e20);
	float c = Math::dot(e20, e20);
	float ac_bb = (a * c) - (b * b);
	Vector3f vp(point.x - pa.x, point.y - pa.y, point.z - pa.z);
	float d = Math::dot(vp, e10);
	float e = Math::dot(vp, e20);
	float x = (d * c) - (e * b);
	float y = (e * a) - (d * b);
	float z = x + y - ac_bb;
	return ((in(z) & ~(in(x) | in(y))) & 0x80000000);
}

//C(t) = basePoint + t * velocity, t [0, 1]
//SignedDistance(p) = N * p + Cp;
//t0 = (1 - SignedDistance(basePoint))/(N dot velocity)
//t1 = -(1 + SignedDistance(basePoint))/(N dot velocity)
// if t0 > 1 || t0 < 0   no intersection
// if t1 > 1 || t1 < 0   no intersection
// planeIntersectionPoint = basePoint - planeNormal + t0 * velocity
// if triangle area >= (tri1 + tri2 + tri3 - 0.001)
//     intersecting
// Ellipsoid and Triangle
for (int i = 0; i < model2.getIndices().size(); i += 3)
{
	Vector3f eSpace = Vector3f(1.0f) / model.getScalar();

	Vector3f tp1 = model2.model * model2.getVertices()[model2.getIndices()[i]].position;
	Vector3f tp2 = model2.model * model2.getVertices()[model2.getIndices()[i + 1]].position;
	Vector3f tp3 = model2.model * model2.getVertices()[model2.getIndices()[i + 2]].position;

	tp1 = eSpace * tp1;
	tp2 = eSpace * tp2;
	tp3 = eSpace * tp3;

	Vector3f triPoint = (tp1 + tp2 + tp3) / 3;

	Vector3f N = Math::normalize(Math::cross(tp2 - tp1, tp3 - tp1));
	Vector3f eBasePoint = eSpace * model.getPosition();
	Vector3f eVelocity = eSpace * velocity;

	float Cp = -Math::dot(N, triPoint);

	bool embedded = false;

	float t0;
	float t1;

	if (Math::dot(N, Math::normalize(eVelocity)) <= 0) {
		if (Math::dot(N, eVelocity) == 0) {
			float d = Math::dot(N, eBasePoint) + Cp;

			if (std::fabs(d) >= 1)
				continue;

			embedded = true;
			t0 = 0.0f;
			t1 = 1.0f;
		}
		else
		{
			t0 = (-1 - (Math::dot(N, eBasePoint) + Cp)) / (Math::dot(N, eVelocity));
			t1 = (1 - (Math::dot(N, eBasePoint) + Cp)) / (Math::dot(N, eVelocity));

			if (t0 > t1)
				std::swap(t0, t1);
			if (t0 > 1.0f || t1 < 0.0f)
				continue;

			t0 = t0 < 0.0 ? 0.0 : t0;
			t1 = t1 < 0.0 ? 0.0 : t1;
			t0 = t0 > 1.0 ? 1.0 : t0;
			t1 = t1 > 1.0 ? 1.0 : t1;
		}

		bool collided = false;
		float t = 1.0f;

		Vector3f collisionPoint;

		if (!embedded) {
			Vector3f intPoint = eBasePoint - N + eVelocity * t0;

			if (checkPointInTriangle(intPoint, tp1, tp2, tp3))
			{
				model2.getMaterial(0).conf.m_Albedo = { 0, 0, 1 };
				collided = true;
				t = t0;
				collisionPoint = intPoint;
				continue;
			}
		}

		float a, b, c, sq, minRoot;

		a = eVelocity.length();
		a *= a;

		// Check vertices

		// Vertex 1
		b = 2.0f * (Math::dot(eVelocity, eBasePoint - tp1));
		c = (eBasePoint - tp1).length();
		c *= c;
		c -= 1.0f;

		if (getMinRoot(a, b, c, t, minRoot))
		{
			collisionPoint = tp1;
			t = minRoot;

			collided = true;
		}

		// Vertex 2
		b = 2.0f * (Math::dot(eVelocity, eBasePoint - tp2));
		c = (eBasePoint - tp2).length();
		c *= c;
		c -= 1.0f;

		if (getMinRoot(a, b, c, t, minRoot))
		{
			collisionPoint = tp2;
			t = minRoot;

			collided = true;
		}

		// Vertex 3
		b = 2.0f * (Math::dot(eVelocity, eBasePoint - tp3));
		c = (eBasePoint - tp3).length();
		c *= c;
		c -= 1.0f;

		if (getMinRoot(a, b, c, t, minRoot))
		{
			collisionPoint = tp3;
			t = minRoot;

			collided = true;
		}

		// Check edges

		Vector3f edge, baseToVertex;
		float edgeSqLength, edgeDotVelocity, edgeDotBaseToVertex, baseToVertexSqLength, velocitySqLength;
		velocitySqLength = eVelocity.length();
		velocitySqLength *= velocitySqLength;

		edge = tp2 - tp1;
		baseToVertex = tp1 - eBasePoint;
		baseToVertexSqLength = baseToVertex.length();
		baseToVertexSqLength *= baseToVertexSqLength;
		edgeSqLength = edge.length();
		edgeSqLength *= edgeSqLength;
		edgeDotVelocity = Math::dot(edge, eVelocity);
		edgeDotBaseToVertex = Math::dot(edge, baseToVertex);

		a = edgeSqLength * -velocitySqLength + edgeDotVelocity * edgeDotVelocity;
		b = edgeSqLength * (2.0f * Math::dot(eVelocity, baseToVertex)) - 2.0f * edgeDotVelocity * edgeDotBaseToVertex;
		c = edgeSqLength * (1 - baseToVertexSqLength) + edgeDotBaseToVertex * edgeDotBaseToVertex;

		// ax^2 + bx + c
		if (getMinRoot(a, b, c, t, minRoot))
		{
			float f = (edgeDotVelocity * minRoot - edgeDotBaseToVertex) / edgeSqLength;
			if (f >= 0.0f && f <= 1.0f)
			{
				collided = true;
				t = minRoot;
				collisionPoint = tp1 + edge * f;
			}
		}

		edge = tp3 - tp2;
		baseToVertex = tp2 - eBasePoint;
		baseToVertexSqLength = baseToVertex.length();
		baseToVertexSqLength *= baseToVertexSqLength;
		edgeSqLength = edge.length();
		edgeSqLength *= edgeSqLength;
		edgeDotVelocity = Math::dot(edge, eVelocity);
		edgeDotBaseToVertex = Math::dot(edge, baseToVertex);

		a = edgeSqLength * -velocitySqLength + edgeDotVelocity * edgeDotVelocity;
		b = edgeSqLength * (2.0f * Math::dot(eVelocity, baseToVertex)) - 2.0f * edgeDotVelocity * edgeDotBaseToVertex;
		c = edgeSqLength * (1 - baseToVertexSqLength) + edgeDotBaseToVertex * edgeDotBaseToVertex;

		// ax^2 + bx + c
		if (getMinRoot(a, b, c, t, minRoot))
		{
			float f = (edgeDotVelocity * minRoot - edgeDotBaseToVertex) / edgeSqLength;
			if (f >= 0.0f && f <= 1.0f)
			{
				collided = true;
				t = minRoot;
				collisionPoint = tp2 + edge * f;
			}
		}

		edge = tp1 - tp3;
		baseToVertex = tp3 - eBasePoint;
		baseToVertexSqLength = baseToVertex.length();
		baseToVertexSqLength *= baseToVertexSqLength;
		edgeSqLength = edge.length();
		edgeSqLength *= edgeSqLength;
		edgeDotVelocity = Math::dot(edge, eVelocity);
		edgeDotBaseToVertex = Math::dot(edge, baseToVertex);

		a = edgeSqLength * -velocitySqLength + edgeDotVelocity * edgeDotVelocity;
		b = edgeSqLength * (2.0f * Math::dot(eVelocity, baseToVertex)) - 2.0f * edgeDotVelocity * edgeDotBaseToVertex;
		c = edgeSqLength * (1 - baseToVertexSqLength) + edgeDotBaseToVertex * edgeDotBaseToVertex;

		// ax^2 + bx + c
		if (getMinRoot(a, b, c, t, minRoot))
		{
			float f = (edgeDotVelocity * minRoot - edgeDotBaseToVertex) / edgeSqLength;
			if (f >= 0.0f && f <= 1.0f)
			{
				collided = true;
				t = minRoot;
				collisionPoint = tp3 + edge * f;
			}
		}

		if (collided)
			model2.getMaterial(0).conf.m_Albedo = { 0, 0, 1 };

	}
}
