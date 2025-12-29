#include "StdAfx.h"
#include "PhysicsObject.h"

const float c_fFrameTime = 0.02f;
const float EPSILON		 = 0.001f;

IPhysicsWorld* IPhysicsWorld::ms_pWorld = NULL;
IObjectManager* IObjectManager::ms_ObjManager = NULL;

void CPhysicsObject::Update(float fElapsedTime)
{
	if (m_xPushingPosition.isPlaying())
		m_xPushingPosition.Interpolate(fElapsedTime);
	if (m_yPushingPosition.isPlaying())
		m_yPushingPosition.Interpolate(fElapsedTime);
}

void CPhysicsObject::Accumulate(D3DXVECTOR3 * pv3Position)
{
	// If object is moving, give minor power to object.
	float fForce = 0.0f;

	if (fabs(m_v3Velocity.x) < EPSILON ||
		fabs(m_v3Velocity.y) < EPSILON ||
		fabs(m_v3Velocity.z) < EPSILON )
	{
		fForce -= (m_fMass * m_fFriction);
	}

	m_v3Acceleration = m_v3Direction * (fForce / m_fMass);
	m_v3Velocity += m_v3Acceleration;
	if (m_v3Velocity.x * m_v3Direction.x < EPSILON)
	{
		m_v3Velocity.x = 0.0f;
		m_v3Direction.x = 0.0f;
	}
	if (m_v3Velocity.y * m_v3Direction.y < EPSILON)
	{
		m_v3Velocity.y = 0.0f;
		m_v3Direction.y = 0.0f;
	}
	if (m_v3Velocity.z * m_v3Direction.z < EPSILON)
	{
		m_v3Velocity.z = 0.0f;
		m_v3Direction.z = 0.0f;
	}

	pv3Position->x += m_v3Velocity.x;
	pv3Position->y += m_v3Velocity.y;
	pv3Position->z += m_v3Velocity.z;
}

void CPhysicsObject::IncreaseExternalForce(const D3DXVECTOR3 & c_rvBasePosition, float fForce)
{
	// Accumulate Acceleration by External Force
	m_v3Acceleration = m_v3Direction * (fForce / m_fMass);
	m_v3Velocity = m_v3Acceleration;
/*
	Tracenf("force %f, mass %f, accel (%f, %f, %f)", fForce, m_fMass,
		m_v3Acceleration.x,
		m_v3Acceleration.y,
		m_v3Acceleration.z);
*/
	// NOTE : 얼마 이동하는지 계산한다. 단지 100번을 크다면? ;
	const int LoopValue = 100;
	D3DXVECTOR3 v3Movement(0.0f, 0.0f, 0.0f);

	for(int i = 0; i < LoopValue; ++i)
	{
		Accumulate(&v3Movement);

		// VICTIM_COLLISION_TEST
		IPhysicsWorld* pWorld = IPhysicsWorld::GetPhysicsWorld();
		if (pWorld)
		{
			if (pWorld->isPhysicalCollision(c_rvBasePosition + v3Movement))
			{
				Initialize();
				return;

				//for (float fRatio = 0.0f; fRatio < 1.0f; fRatio += 0.1f)
				//{
				//	// 점점 가깝게 체크한다
				//	if (pWorld->isPhysicalCollision(c_rvBasePosition + v3Movement * fRatio))
				//	{
				//		v3Movement = D3DXVECTOR3 (0.0f, 0.0f, 0.0f);
				//		break;
				//	}
				//}
				//break;
			}
		}
		// VICTIM_COLLISION_TEST_END

		if (fabs(m_v3Velocity.x) < EPSILON &&
			fabs(m_v3Velocity.y) < EPSILON &&
			fabs(m_v3Velocity.z) < EPSILON )
			break;
	}

	// FIXED: Need to get current position from ActorInstance
	// This will be passed from the calling function now
	TPixelPosition currentPos;
	currentPos.x = c_rvBasePosition.x;
	currentPos.y = c_rvBasePosition.y;
	currentPos.z = c_rvBasePosition.z;

	TPixelPosition deltaPos;
	deltaPos.x = v3Movement.x;
	deltaPos.y = v3Movement.y;
	deltaPos.z = v3Movement.z;

	SetLastPosition(currentPos, deltaPos, float(LoopValue) * c_fFrameTime);

	if( m_pActorInstance )
	{
		IObjectManager* pObjectManager = IObjectManager::GetObjectManager();
		pObjectManager->AdjustCollisionWithOtherObjects( m_pActorInstance );
	}
}

// ============================================================
// FIXED: Modified signature - now takes current + delta
// ============================================================
void CPhysicsObject::SetLastPosition(const TPixelPosition & c_rCurrentPosition,
                                      const TPixelPosition & c_rDeltaPosition,
                                      float fBlendingTime)
{
	// Calculate FINAL destination (current + delta)
	m_v3FinalPosition.x = float(c_rCurrentPosition.x + c_rDeltaPosition.x);
	m_v3FinalPosition.y = float(c_rCurrentPosition.y + c_rDeltaPosition.y);
	m_v3FinalPosition.z = float(c_rCurrentPosition.z + c_rDeltaPosition.z);

	// Store delta separately
	m_v3DeltaPosition.x = float(c_rDeltaPosition.x);
	m_v3DeltaPosition.y = float(c_rDeltaPosition.y);
	m_v3DeltaPosition.z = float(c_rDeltaPosition.z);

	// Setup interpolation with delta values
	m_xPushingPosition.Setup(0.0f, c_rDeltaPosition.x, fBlendingTime);
	m_yPushingPosition.Setup(0.0f, c_rDeltaPosition.y, fBlendingTime);
}

// ============================================================
// GetLastPosition - returns DELTA (original behavior maintained)
// ============================================================
void CPhysicsObject::GetLastPosition(TPixelPosition * pPosition)
{
	pPosition->x = (m_v3DeltaPosition.x);
	pPosition->y = (m_v3DeltaPosition.y);
	pPosition->z = (m_v3DeltaPosition.z);
}

// ============================================================
// NEW: Get final destination position (for GetBlendingPosition fix)
// ============================================================
void CPhysicsObject::GetFinalPosition(TPixelPosition * pPosition)
{
	pPosition->x = m_v3FinalPosition.x;
	pPosition->y = m_v3FinalPosition.y;
	pPosition->z = m_v3FinalPosition.z;
}

// ============================================================
// NEW: Get delta movement (for compatibility)
// ============================================================
void CPhysicsObject::GetDeltaPosition(TPixelPosition * pPosition)
{
	pPosition->x = m_v3DeltaPosition.x;
	pPosition->y = m_v3DeltaPosition.y;
	pPosition->z = m_v3DeltaPosition.z;
}

void CPhysicsObject::SetDirection(const D3DXVECTOR3 & c_rv3Direction)
{
	m_v3Direction.x = c_rv3Direction.x;
	m_v3Direction.y = c_rv3Direction.y;
	m_v3Direction.z = c_rv3Direction.z;
}

float CPhysicsObject::GetXMovement()
{
	return m_xPushingPosition.GetChangingValue();
}

float CPhysicsObject::GetYMovement()
{
	return m_yPushingPosition.GetChangingValue();
}

void CPhysicsObject::ClearSavingMovement()
{
	m_xPushingPosition.Initialize();
	m_yPushingPosition.Initialize();
}

bool CPhysicsObject::isBlending()
{
	// NOTE : IncreaseExternalForce() 에 의한 밀림을 처리중인가?
	if (0.0f != D3DXVec3Length(&m_v3Velocity))
		return true;

	// NOTE : SetLastPosition() 에 의한 밀림을 처리중인가?
	if (m_xPushingPosition.isPlaying() ||
		m_yPushingPosition.isPlaying())
		return true;

	return false;
}

void CPhysicsObject::Initialize()
{
	m_fMass = 1.0f;
	m_fFriction = 0.3f;
	m_v3Direction = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_v3Acceleration = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_v3Velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	// FIXED: Initialize both delta and final positions
	m_v3DeltaPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_v3FinalPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	m_xPushingPosition.Initialize();
	m_yPushingPosition.Initialize();
}

CPhysicsObject::CPhysicsObject()
{
	m_pActorInstance = NULL;
}

CPhysicsObject::~CPhysicsObject()
{
}
