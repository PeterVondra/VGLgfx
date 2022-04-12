for (int i = 0; i < test.getVertices().size(); i += 3)
{
	Vector3f triPoint1 = model.getModelMatrix() * (model.getVertices()[i].position + model.getVertices()[i + 1].position + model.getVertices()[i + 2].position) / 3;

	Vector3f nrm1 = Math::normalize(Math::cross(model.getVertices()[i + 1].position - model.getVertices()[i].position, model.getVertices()[i + 2].position - model.getVertices()[i].position));;

	float D1 = -Math::dot(nrm1, triPoint1);

	Vector3f tp1 = model.getModelMatrix() * model.getVertices()[i + 2].position;
	Vector3f tp2 = model.getModelMatrix() * model.getVertices()[i + 1].position;
	Vector3f tp3 = model.getModelMatrix() * model.getVertices()[i].position;

	float area = getTriArea(tp1, tp2, tp3);

	for (int j = 0; j < test.getVertices().size(); j += 3)
	{

		Vector3f p1 = test.getModelMatrix() * test.getVertices()[j].position;
		Vector3f p2 = test.getModelMatrix() * test.getVertices()[j + 1].position;
		Vector3f p3 = test.getModelMatrix() * test.getVertices()[j + 2].position;

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

			if (area >= (area1 - 0.001f))
				intersect = true;
		}
		else if ((t2 <= 1 && t2 >= 0))
		{
			float area1 = getTriArea(int2, tp1, tp2) + getTriArea(int2, tp1, tp3) + getTriArea(int2, tp2, tp3);

			if (area >= (area1 - 0.001f))
				intersect = true;
		}
		else if ((t3 <= 1 && t3 >= 0))
		{
			float area1 = getTriArea(int3, tp1, tp2) + getTriArea(int3, tp1, tp3) + getTriArea(int3, tp2, tp3);

			if (area >= (area1 - 0.001f))
				intersect = true;
		}
		else
			intersect = false;
	}
}