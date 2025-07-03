#include "ChessObject.h"

// ChessObject::ChessObject()
// {
//}

ChessObject::~ChessObject()
{
}

Mat44 ChessObject::GetModelToWorldTransform() const
{
    Mat44 rotate;
    rotate = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
    Mat44 translate;
    translate = Mat44::MakeTranslation3D(m_position);

    translate.Append(rotate); 
    return translate;
}

void ChessObject::OnImpacted()
{
    m_isImpacted = true;
}

void ChessObject::OnUnImpacted()
{
    m_isImpacted = false;
}

void ChessObject::OnUnGrabbed()
{
    m_isGrabbed = false;
}

void ChessObject::OnGrabbed()
{
    m_isGrabbed = true;
}
