#include "BoxColliderSystem.h"
#include "RigidBodyComponent.h"
#include "RigidBodySystem.h"

namespace Reality
{
	BoxColliderSystem::BoxColliderSystem(rp3d::CollisionWorld& _rp3dWorld)
		:rp3dWorld(_rp3dWorld)
	{
		requireComponent<BoxColliderComponent>();
	}

	void BoxColliderSystem::Update(float deltaTime)
	{
		std::vector<rp3d::ProxyShape*> rp3dShapesTemp;
		std::vector<int> aliveIds;
		aliveIds.resize(rp3dShapes.size());
		int id = 0;

		for (auto e : getEntities())
		{
			auto &boxCollider = e.getComponent<BoxColliderComponent>();

			if (boxCollider.body.hasComponent<RigidBodyComponent>())
			{
				auto &body = boxCollider.body.getComponent<RigidBodyComponent>();

				// Update RP3D Ids
				// Calculate local rp3d transform
				rp3d::Vector3 initPosition(boxCollider.offset.x,
					boxCollider.offset.y,
					boxCollider.offset.z);
				rp3d::Quaternion initOrientation = rp3d::Quaternion(boxCollider.orientation.x, boxCollider.orientation.y, boxCollider.orientation.z, boxCollider.orientation.w);
				rp3d::Transform rp3dtransform(initPosition, initOrientation);

				auto rp3dBody = getWorld().getSystemManager().getSystem<RigidBodySystem>().rp3dBodies[body.rp3dId];
				// If new rigidbody, create an entry
				if (boxCollider.rp3dId < 0)
				{
					rp3d::BoxShape* shape = new rp3d::BoxShape(rp3d::Vector3(boxCollider.size.x, boxCollider.size.y, boxCollider.size.z) * 0.5f);
					// Add the collision shape to the rigid body

					rp3d::ProxyShape * proxyShape = rp3dBody->addCollisionShape(shape, rp3dtransform);
					proxyShape->setUserData(&boxCollider);
					rp3dShapesTemp.push_back(proxyShape);
					boxCollider.rp3dId = id;
				}
				else
				{
					rp3d::ProxyShape * shape = rp3dShapes[boxCollider.rp3dId];
					shape->setLocalToBodyTransform(rp3dtransform);
					aliveIds[boxCollider.rp3dId] = 1;
					rp3dShapesTemp.push_back(shape);
					boxCollider.rp3dId = id;
				}
			}
			id++;
			auto& bodyTransform = boxCollider.body.getComponent<TransformComponentV2>();
			getWorld().data.renderUtil->DrawCube(bodyTransform.GetUnScaledTransformationMatrix() * Vector4(boxCollider.offset, 1.0f), boxCollider.size, bodyTransform.GetOrientation() * boxCollider.orientation);
		}

		for (int i = 0; i < aliveIds.size(); i++)
		{
			if (aliveIds[i] == 0)
			{
				auto shape = rp3dShapes[i]->getCollisionShapePublic();
				rp3dShapes[i]->getBody()->removeCollisionShape(rp3dShapes[i]);
				delete shape;
			}
		}

		rp3dShapes = rp3dShapesTemp;
	}
}
