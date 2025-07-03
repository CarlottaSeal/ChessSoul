#include "ChessPieceDefinition.h"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

std::vector<ChessPieceDefinition> ChessPieceDefinition::s_chessPieceDefs;

extern Renderer* g_theRenderer;

ChessPieceDefinition::ChessPieceDefinition(XmlElement const& element)
{
    m_name = ParseXmlAttribute(element, "name", "UNKNOWN PAWN NAME");
    
    std::string shaderName = ParseXmlAttribute(element, "shaderName", "UNKNOWN SHADER NAME");
    m_shader = g_theRenderer->CreateOrGetShader(shaderName.c_str(), VertexType::VERTEX_PCUTBN);

    std::string diffuseTextureBlack = ParseXmlAttribute(element, "diffuseTextureNameForBlack", "UNKNOWN DIFFUSE BLACK NAME");
    std::string diffuseTextureWhite = ParseXmlAttribute(element, "diffuseTextureNameForWhite", "UNKNOWN DIFFUSE WHITE NAME");
    m_diffuseTextures[0] = g_theRenderer->CreateOrGetTextureFromFile(diffuseTextureBlack.c_str());
    m_diffuseTextures[1] = g_theRenderer->CreateOrGetTextureFromFile(diffuseTextureWhite.c_str());

    std::string normalTextureBlack = ParseXmlAttribute(element, "normalTextureNameForBlack", "UNKNOWN DIFFUSE BLACK NAME");
    std::string normalTextureWhite = ParseXmlAttribute(element, "normalTextureNameForWhite", "UNKNOWN DIFFUSE WHITE NAME");
    m_normalTextures[0] = g_theRenderer->CreateOrGetTextureFromFile(normalTextureBlack.c_str());
    m_normalTextures[1] = g_theRenderer->CreateOrGetTextureFromFile(normalTextureWhite.c_str());

    std::string specTextureBlack = ParseXmlAttribute(element, "specTextureNameForBlack", "UNKNOWN DIFFUSE BLACK NAME");
    std::string specTextureWhite = ParseXmlAttribute(element, "specTextureNameForWhite", "UNKNOWN DIFFUSE WHITE NAME");
    m_specGlossEmitTextures[0] = g_theRenderer->CreateOrGetTextureFromFile(specTextureBlack.c_str());
    m_specGlossEmitTextures[1] = g_theRenderer->CreateOrGetTextureFromFile(specTextureWhite.c_str());
    
    m_type = GetChessPieceTypeByName(m_name);
    InitializeVertsAndBuffersForType(m_type);

    InitializeGlyphs(m_type);
}

void ChessPieceDefinition::InitializeChessPieceDefinitions()
{
    XmlDocument chessDefDoc;
    XmlResult weaponLoadResult = chessDefDoc.LoadFile("Data/Definitions/ChessDefinitions.xml");
    if (weaponLoadResult != XmlResult::XML_SUCCESS)
    {
        ERROR_AND_DIE(Stringf("Cannot load chess definitions!"));
    }

    XmlElement* chessRootElement = chessDefDoc.RootElement();
    XmlElement* chessFirstElement = chessRootElement->FirstChildElement();

    while (chessFirstElement)
    {
        ChessPieceDefinition weaponDef(*chessFirstElement);
        s_chessPieceDefs.push_back(weaponDef);

        chessFirstElement = chessFirstElement->NextSiblingElement();
    }
}

void ChessPieceDefinition::ClearDefinitions()
{
    for (ChessPieceDefinition chessPieceDef : s_chessPieceDefs)
    {
        delete chessPieceDef.m_indexBuffers[0];
        delete chessPieceDef.m_indexBuffers[1];
        chessPieceDef.m_indexBuffers[0] = nullptr;
        chessPieceDef.m_indexBuffers[1] = nullptr;
        delete chessPieceDef.m_vertexBuffers[0];
        delete chessPieceDef.m_vertexBuffers[1];
        chessPieceDef.m_vertexBuffers[0] = nullptr;
        chessPieceDef.m_vertexBuffers[1] = nullptr;
    }
    s_chessPieceDefs.clear();
}

ChessPieceDefinition const& ChessPieceDefinition::GetChessPieceDefinitionByName(std::string const& name)
{
    for (int defIndex = 0; defIndex < s_chessPieceDefs.size(); defIndex++)
    {
        if (s_chessPieceDefs[defIndex].m_name == name)
        {
            return s_chessPieceDefs[defIndex];
        }
    }
    ERROR_AND_DIE(Stringf("Unkown ChessDef \"%s\"!", name.c_str()));
}

ChessPieceDefinition const& ChessPieceDefinition::GetChessPieceDefinitionByChessPieceType(ChessPieceType type)
{
    std::string name;
    if (type == ChessPieceType::Bishop)
    {
        name = "bishop";
    }
    else if (type == ChessPieceType::King)
    {
        name = "king";
    }
    else if (type == ChessPieceType::Queen)
    {
        name = "queen";
    }
    else if (type == ChessPieceType::Knight)
    {
        name = "knight";
    }
    else if (type == ChessPieceType::Rook)
    {
        name = "rook";
    }
    else if (type == ChessPieceType::Pawn)
    {
        name = "pawn";
    }
    return GetChessPieceDefinitionByName(name);
}

ChessPieceType ChessPieceDefinition::GetChessPieceTypeByName(std::string const& name) const
{
    if (name == "bishop")
        return ChessPieceType::Bishop;
    else if (name == "king")
        return ChessPieceType::King;
    else if (name == "queen")
        return ChessPieceType::Queen;
    else if (name == "knight")
        return ChessPieceType::Knight;
    else if (name == "rook")
        return ChessPieceType::Rook;
    else if (name == "pawn")
        return ChessPieceType::Pawn;
    else
        ERROR_AND_DIE("UNKNOWN ChessPieceType");
}

void ChessPieceDefinition::InitializeVertsAndBuffersForType(ChessPieceType type)
{
    std::vector<Vertex_PCUTBN> vertsForBlack;
    //std::vector<unsigned int> m_indicesForBlack;
    std::vector<Vertex_PCUTBN> vertsForWhite;
    //std::vector<unsigned int> m_indicesForWhite;
    if (type == ChessPieceType::Bishop)
    {
        AddVertsForIndexCylinderZ3D(vertsForBlack, m_indicesForBlack, Vec2(0.f, 0.f), FloatRange(0.f, 0.1f), 0.3f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForBlack, m_indicesForBlack, Vec2(0.f, 0.f), FloatRange(0.1f, 0.5f), 0.27f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForBlack, m_indicesForBlack, Vec2(0.f, 0.f), FloatRange(0.5f, 0.6f), 0.3f,
                                    Rgba8::WHITE);
        AddVertsForIndexSphere3D(vertsForBlack, m_indicesForBlack, Vec3(0.f,0.f,0.7f), 0.14f, 16, 8 , Rgba8::WHITE);

        AddVertsForIndexCylinderZ3D(vertsForWhite, m_indicesForWhite, Vec2(0.f, 0.f), FloatRange(0.f, 0.1f), 0.3f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForWhite, m_indicesForWhite, Vec2(0.f, 0.f), FloatRange(0.1f, 0.5f), 0.27f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForWhite, m_indicesForWhite, Vec2(0.f, 0.f), FloatRange(0.5f, 0.6f), 0.3f,
                                    Rgba8::WHITE);
        AddVertsForIndexSphere3D(vertsForWhite, m_indicesForWhite, Vec3(0.f,0.f,0.7f), 0.14f, 16, 8 , Rgba8::WHITE);
    }
    else if (type == ChessPieceType::King)
    {
        AddVertsForIndexCylinderZ3D(vertsForBlack, m_indicesForBlack, Vec2(0.f, 0.f), FloatRange(0.f, 0.1f), 0.35f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForBlack, m_indicesForBlack, Vec2(0.f, 0.f), FloatRange(0.1f, 0.7f), 0.3f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForBlack, m_indicesForBlack, Vec2(0.f, 0.f), FloatRange(0.7f, 0.8f), 0.35f,
                                    Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForBlack, m_indicesForBlack,
                               AABB3(Vec3(-0.05f, -0.25f, 0.8f), Vec3(0.05f, 0.25f, 0.9f)), Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForBlack, m_indicesForBlack,
                               AABB3(Vec3(-0.25f, -0.05f, 0.8f), Vec3(0.25f, 0.05f, 0.9f)), Rgba8::WHITE);

        AddVertsForIndexCylinderZ3D(vertsForWhite, m_indicesForWhite, Vec2(0.f, 0.f), FloatRange(0.f, 0.1f), 0.35f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForWhite, m_indicesForWhite, Vec2(0.f, 0.f), FloatRange(0.1f, 0.7f), 0.3f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForWhite, m_indicesForWhite, Vec2(0.f, 0.f), FloatRange(0.7f, 0.8f), 0.35f,
                                    Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForWhite, m_indicesForWhite,
                               AABB3(Vec3(-0.05f, -0.25f, 0.8f), Vec3(0.05f, 0.25f, 0.9f)), Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForWhite, m_indicesForWhite,
                               AABB3(Vec3(-0.25f, -0.05f, 0.8f), Vec3(0.25f, 0.05f, 0.9f)), Rgba8::WHITE);
    }
    else if (type == ChessPieceType::Queen)
    {
        AddVertsForIndexCylinderZ3D(vertsForBlack, m_indicesForBlack, Vec2(0.f, 0.f), FloatRange(0.f, 0.1f), 0.35f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForBlack, m_indicesForBlack, Vec2(0.f, 0.f), FloatRange(0.1f, 0.7f), 0.23f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForBlack, m_indicesForBlack, Vec2(0.f, 0.f), FloatRange(0.7f, 0.8f), 0.35f,
                                    Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForBlack, m_indicesForBlack,
                               AABB3(Vec3(-0.05f, -0.25f, 0.8f), Vec3(0.05f, 0.25f, 0.9f)), Rgba8::WHITE);
        AddVertsForIndexSphere3D(vertsForBlack, m_indicesForBlack, Vec3(0.f,0.f,0.95f), 0.07f, 8,4,Rgba8::WHITE);

        AddVertsForIndexCylinderZ3D(vertsForWhite, m_indicesForWhite, Vec2(0.f, 0.f), FloatRange(0.f, 0.1f), 0.35f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForWhite, m_indicesForWhite, Vec2(0.f, 0.f), FloatRange(0.1f, 0.7f), 0.23f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForWhite, m_indicesForWhite, Vec2(0.f, 0.f), FloatRange(0.7f, 0.8f), 0.35f,
                                    Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForWhite, m_indicesForWhite,
                               AABB3(Vec3(-0.05f, -0.25f, 0.8f), Vec3(0.05f, 0.25f, 0.9f)), Rgba8::WHITE);
        AddVertsForIndexSphere3D(vertsForWhite, m_indicesForWhite, Vec3(0.f,0.f,0.95f), 0.07f, 8,4,Rgba8::WHITE);
    }
    else if (type == ChessPieceType::Knight)
    {
        //AddVertsForIndexCylinder3D(vertsForBlack, m_indicesForBlack, Vec3(), Vec3(0.f,0.f,0.1f), 0.3f,
        //   Rgba8::BLUE);
        AddVertsForIndexCylinderZ3D(vertsForBlack, m_indicesForBlack, Vec2(), FloatRange(0.f,0.1f), 0.3f,
           Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForBlack, m_indicesForBlack, AABB3(Vec3(-0.22f,-0.22f,0.1f), Vec3(0.22f,0.22f,0.4f)),
            Rgba8::WHITE);
        AddVertsForIndexOBB3D(vertsForBlack, m_indicesForBlack, OBB3(Vec3(-0.05f, 0.f, 0.45f),
            Vec3(1.f, 0.f, -0.2f).GetNormalized(), Vec3(0.0f, -0.8f, 0.f), Vec3(0.25f,0.f,1.f).GetNormalized()
            , Vec3(0.1f, 0.1f, 0.2f)),Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForBlack, m_indicesForBlack, AABB3(Vec3(0.12f,-0.05f,0.4f), Vec3(0.22f,0.05f,0.45f)),
            Rgba8::WHITE);

        //AddVertsForIndexCylinder3D(vertsForWhite, m_indicesForWhite, Vec3(), Vec3(0.f,0.f,0.1f), 0.3f,
        //    Rgba8::YELLOW);
        AddVertsForIndexCylinderZ3D(vertsForWhite, m_indicesForWhite, Vec2(), FloatRange(0.f,0.1f), 0.3f,
            Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForWhite, m_indicesForWhite, AABB3(Vec3(-0.22f,-0.22f,0.1f), Vec3(0.22f,0.22f,0.4f)),
            Rgba8::WHITE);
        AddVertsForIndexOBB3D(vertsForWhite, m_indicesForWhite, OBB3(Vec3(-0.05f, 0.f, 0.45f),
            Vec3(1.f, 0.f, -0.2f).GetNormalized(), Vec3(0.0f, -0.8f, 0.f), Vec3(0.25f,0.f,1.f).GetNormalized()
            , Vec3(0.1f, 0.1f, 0.2f)),Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForWhite, m_indicesForWhite, AABB3(Vec3(0.12f,-0.05f,0.4f), Vec3(0.22f,0.05f,0.45f)),
            Rgba8::WHITE);
    }
    else if (type == ChessPieceType::Rook)
    {
        //AddVertsForIndexCylinder3D(vertsForBlack, m_indicesForBlack, Vec3(), Vec3(0.f,0.f,0.1f), 0.35f,
        //    Rgba8::BLUE);
        AddVertsForIndexCylinderZ3D(vertsForBlack, m_indicesForBlack, Vec2(), FloatRange(0.f,0.1f), 0.35f,
            Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForBlack, m_indicesForBlack, AABB3(Vec3(-0.2f,-0.2f,0.1f), Vec3(0.2f,0.2f,0.4f)),
            Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForBlack, m_indicesForBlack, AABB3(Vec3(-0.22f,-0.22f,0.4f), Vec3(0.22f,0.22f,0.5f)),
            Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForBlack, m_indicesForBlack, AABB3(Vec3(-0.17f,-0.17f,0.5f), Vec3(0.17f,0.17f,0.6f)),
            Rgba8::WHITE);

        //AddVertsForIndexCylinder3D(vertsForWhite, m_indicesForWhite, Vec3(), Vec3(0.f,0.f,0.1f), 0.35f,
        //    Rgba8::YELLOW);
        AddVertsForIndexCylinderZ3D(vertsForWhite, m_indicesForWhite, Vec2(), FloatRange(0.f,0.1f), 0.35f,
            Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForWhite, m_indicesForWhite, AABB3(Vec3(-0.2f,-0.2f,0.1f), Vec3(0.2f,0.2f,0.4f)),
            Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForWhite, m_indicesForWhite, AABB3(Vec3(-0.22f,-0.22f,0.4f), Vec3(0.22f,0.22f,0.5f)),
            Rgba8::WHITE);
        AddVertsForIndexAABB3D(vertsForWhite, m_indicesForWhite, AABB3(Vec3(-0.17f,-0.17f,0.5f), Vec3(0.17f,0.17f,0.6f)),
            Rgba8::WHITE);
    }
    else if (type == ChessPieceType::Pawn)
    {
        AddVertsForIndexCylinderZ3D(vertsForBlack, m_indicesForBlack, Vec2(0.f, 0.f), FloatRange(0.f, 0.1f), 0.25f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForBlack, m_indicesForBlack, Vec2(0.f, 0.f), FloatRange(0.1f, 0.5f), 0.1f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForBlack, m_indicesForBlack, Vec2(0.f, 0.f), FloatRange(0.5f, 0.6f), 0.15f,
                                    Rgba8::WHITE);
        AddVertsForIndexSphere3D(vertsForBlack, m_indicesForBlack, Vec3(0.f,0.f,0.6f), 0.12f, 16, 8 , Rgba8::WHITE);

        AddVertsForIndexCylinderZ3D(vertsForWhite, m_indicesForWhite, Vec2(0.f, 0.f), FloatRange(0.f, 0.1f), 0.25f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForWhite, m_indicesForWhite, Vec2(0.f, 0.f), FloatRange(0.1f, 0.5f), 0.1f,
                                    Rgba8::WHITE);
        AddVertsForIndexCylinderZ3D(vertsForWhite, m_indicesForWhite, Vec2(0.f, 0.f), FloatRange(0.5f, 0.6f), 0.15f,
                                    Rgba8::WHITE);
        AddVertsForIndexSphere3D(vertsForWhite, m_indicesForWhite, Vec3(0.f,0.f,0.6f), 0.12f, 16, 8 , Rgba8::WHITE);
    }

    m_indexes[0] = m_indicesForBlack;
    m_indexes[1] = m_indicesForWhite;
    
    m_vertexBuffers[0] = g_theRenderer->CreateVertexBuffer((unsigned int)vertsForBlack.size() * sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
    m_indexBuffers[0] = g_theRenderer->CreateIndexBuffer((unsigned int)m_indicesForBlack.size()* sizeof(unsigned int), sizeof(unsigned int));

    g_theRenderer->CopyCPUToGPU(vertsForBlack.data(), (unsigned int)(vertsForBlack.size() * sizeof(Vertex_PCUTBN)), m_vertexBuffers[0]);
    g_theRenderer->CopyCPUToGPU(m_indicesForBlack.data(), (unsigned int)(m_indicesForBlack.size() * sizeof(unsigned int)), m_indexBuffers[0]);

    m_vertexBuffers[1] = g_theRenderer->CreateVertexBuffer((unsigned int)vertsForWhite.size() * sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
    m_indexBuffers[1] = g_theRenderer->CreateIndexBuffer((unsigned int)m_indicesForWhite.size()* sizeof(unsigned int), sizeof(unsigned int));

    g_theRenderer->CopyCPUToGPU(vertsForWhite.data(), (unsigned int)(vertsForWhite.size() * sizeof(Vertex_PCUTBN)), m_vertexBuffers[1]);
    g_theRenderer->CopyCPUToGPU(m_indicesForWhite.data(), (unsigned int)(m_indicesForWhite.size() * sizeof(unsigned int)), m_indexBuffers[1]);
}

void ChessPieceDefinition::InitializeGlyphs(ChessPieceType type)
{
    if (type == ChessPieceType::Pawn)
    {
        m_glyph[0] = 'p';
        m_glyph[1] = 'P';
    }
    if (type == ChessPieceType::King)
    {
        m_glyph[0] = 'k';
        m_glyph[1] = 'K';
    }
    if (type == ChessPieceType::Knight)
    {
        m_glyph[0] = 'n';
        m_glyph[1] = 'N';
    }
    if (type == ChessPieceType::Bishop)
    {
        m_glyph[0] = 'b';
        m_glyph[1] = 'B';
    }
    if (type == ChessPieceType::Rook)
    {
        m_glyph[0] = 'r';
        m_glyph[1] = 'R';
    }
    if (type == ChessPieceType::Queen)
    {
        m_glyph[0] = 'q';
        m_glyph[1] = 'Q';
    }
}
