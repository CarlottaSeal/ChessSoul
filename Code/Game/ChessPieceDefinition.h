#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/Shader.hpp"

#include "Gamecommon.hpp"

class ChessPieceDefinition
{
public:
    ChessPieceDefinition(XmlElement const& element);

public:
    static void InitializeChessPieceDefinitions();
    static void ClearDefinitions();

    static ChessPieceDefinition const& GetChessPieceDefinitionByName(std::string const& name);
    static ChessPieceDefinition const& GetChessPieceDefinitionByChessPieceType(ChessPieceType type);

    ChessPieceType GetChessPieceTypeByName(std::string const& name) const;
    void InitializeVertsAndBuffersForType(ChessPieceType type);
    void InitializeGlyphs(ChessPieceType type);
    
public:
    static std::vector<ChessPieceDefinition> s_chessPieceDefs;

public:
    std::string m_name;
    ChessPieceType m_type;
    char m_glyph[2];

    IndexBuffer* m_indexBuffers[2];
    VertexBuffer* m_vertexBuffers[2];
    //std::vector<unsigned int>* m_indexes[2];
    std::vector<unsigned int> m_indexes[2];
    
    std::vector<unsigned int> m_indicesForBlack;
    std::vector<unsigned int> m_indicesForWhite;

    Shader* m_shader = nullptr;

    Texture* m_diffuseTextures[2];
    Texture* m_normalTextures[2];
    Texture* m_specGlossEmitTextures[2];
};

