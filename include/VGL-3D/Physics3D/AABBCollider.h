Vector3f dist1 = cube2.getPosition() - cube2.getSize() / 2 - cube.getPosition() - cube.getSize() / 2;
Vector3f dist2 = cube.getPosition() - cube.getSize() / 2 - cube2.getPosition() - cube2.getSize() / 2;
Vector3f dist = { std::max(dist1.x, dist2.x), std::max(dist1.y, dist2.y), std::max(dist1.z, dist2.z) };
float maxDist = std::max(std::max(dist.x, dist.y), dist.z);

if (maxDist < 0)
	std::cout << "lol" << std::endl;