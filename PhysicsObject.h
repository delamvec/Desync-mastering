#pragma once

#include "MapUtil.h"

class IPhysicsWorld
{
public:
	IPhysicsWorld()
	{
		assert(ms_pWorld == NULL);
		ms_pWorld = this;
	}

	virtual ~IPhysicsWorld()
	{
		if (this == ms_pWorld)
			ms_pWorld = NULL;
	}

	static IPhysicsWorld* GetPhysicsWorld()
	{
		return ms_pWorld;
	}

	virtual bool isPhysicalCollision(const D3DXVECTOR3 & c_rvCheckPosition) = 0;

private:
	static IPhysicsWorld* ms_pWorld;
};

class CActorInstance;
//NOTE : 이게 안되어야 Dependency Inversion이 되고 구조가 괜찮은 interface로 나뉘기 위해서 이미인터 만듬
//나중에 리팩토링을 한다면 world나 live object관리 책임은 base project에서 (gamelib이나 더 base project에서 해야 한다 )
class IObjectManager
{
public:
	IObjectManager()
	{
		assert(ms_ObjManager == NULL);
		ms_ObjManager = this;
	}

	virtual ~IObjectManager()
	{
		if (this == ms_ObjManager)
			ms_ObjManager = NULL;
	}

	static IObjectManager* GetObjectManager()
	{
		return ms_ObjManager;
	}

	virtual void AdjustCollisionWithOtherObjects(CActorInstance* pInst ) = 0;

private:
	static IObjectManager* ms_ObjManager;
};

class CPhysicsObject
{
	public:
		CPhysicsObject();
		virtual ~CPhysicsObject();

		void Initialize();

		void Update(float fElapsedTime);

		bool isBlending();

		void SetDirection(const D3DXVECTOR3 & c_rv3Direction);
		void IncreaseExternalForce(const D3DXVECTOR3 & c_rvBasePosition, float fForce);

		// FIXED: Modified signature - now takes current position + delta
		void SetLastPosition(const TPixelPosition & c_rCurrentPosition,
		                     const TPixelPosition & c_rDeltaPosition,
		                     float fBlendingTime);

		// Original GetLastPosition - returns DELTA
		void GetLastPosition(TPixelPosition * pPosition);

		// NEW: Get final destination position (for GetBlendingPosition fix)
		void GetFinalPosition(TPixelPosition * pPosition);

		// NEW: Get delta movement (for compatibility)
		void GetDeltaPosition(TPixelPosition * pPosition);

		float GetXMovement();
		float GetYMovement();
		void ClearSavingMovement();

		void SetActorInstance( CActorInstance* pInst ) { m_pActorInstance = pInst; }
		CActorInstance* GetActorInstance() { return m_pActorInstance; }

	protected:
		void Accumulate(D3DXVECTOR3 * pv3Position);

	protected:
		float m_fMass;
		float m_fFriction;
		D3DXVECTOR3 m_v3Direction;
		D3DXVECTOR3 m_v3Acceleration;
		D3DXVECTOR3 m_v3Velocity;

		// Renamed from m_v3LastPosition to be more clear
		D3DXVECTOR3 m_v3DeltaPosition;  // Movement delta

		// NEW: Store final destination position
		D3DXVECTOR3 m_v3FinalPosition;  // Final position after blend completes

		CEaseOutInterpolation m_xPushingPosition;
		CEaseOutInterpolation m_yPushingPosition;

		CActorInstance*	m_pActorInstance;
};
