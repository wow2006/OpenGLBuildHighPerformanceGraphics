// STL
#include <map>
#include <string>
#include <fstream>
// Internal
#include "Ezm.hpp"
#include "MeshImport.h"
// 3rdParty
#include "3rdParty/pugi_xml/pugixml.hpp"
#include "3rdParty/pugi_xml/pugiconfig.hpp"

NVSHARE::MeshSystemContainer *msc;
NVSHARE::MeshImport *meshImportLibrary;

std::string trim(const std::string &str,
                 const std::string &whitespace = " \t") {
  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == std::string::npos) {
    return ""; // no content
  }

  const auto strEnd = str.find_last_not_of(whitespace);
  const auto strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

bool EzmLoader::Load(const std::string &filename,
                     std::vector<SubMesh> &submeshes,
                     std::vector<Vertex> &vertices,
                     std::vector<unsigned short> &indices,
                     std::map<std::string, std::string> &materialNames,
                     glm::vec3 &min, glm::vec3 &max) {
  min = glm::vec3(1000.0f);
  max = glm::vec3(-1000.0f);

  pugi::xml_document doc;
  pugi::xml_parse_result res = doc.load_file(filename.c_str());

  // load material
  pugi::xml_node mats = doc.child("MeshSystem").child("Materials");
  int totalMaterials =
      atoi(mats.attribute("count").value()); // ms->mMaterialCount;
  pugi::xml_node material = mats.child("Material");
  for (int i = 0; i < totalMaterials; i++) {
    std::string name = material.attribute("name").value();
    std::string metadata = material.attribute("meta_data").value();
    if (metadata.find("%") != std::string::npos) {
      int startGarbage = metadata.find_first_of("%20");
      int endGarbage = metadata.find_last_of("%20");

      if (endGarbage != std::string::npos)
        metadata = metadata.erase(endGarbage - 2, 3);
      if (startGarbage != std::string::npos)
        metadata = metadata.erase(startGarbage, 3);
    }
    int diffuseIndex = metadata.find("diffuse=");
    if (diffuseIndex != std::string::npos)
      metadata = metadata.erase(diffuseIndex, strlen("diffuse="));
    int len = metadata.length();
    if (len > 0) {

      std::string fullName = "";
      // add the material to the material map

      // get filename only
      int index = metadata.find_last_of("\\");

      if (index == std::string::npos) {
        fullName.append(metadata);
      } else {
        std::string fileNameOnly =
            metadata.substr(index + 1, metadata.length());
        fullName.append(fileNameOnly);
      }
      // see if the material exists already?
      bool exists = true;
      if (materialNames.find(name) == materialNames.end())
        exists = false;

      if (!exists)
        materialNames[name] = (fullName);

    } else {
      bool exists = true;
      if (materialNames.find(name) == materialNames.end())
        exists = false;

      if (!exists)
        materialNames[name] = "";
    }
    material = material.next_sibling("Material");
  }

  // load meshes using MeshImport library
  meshImportLibrary = NVSHARE::loadMeshImporters(".");
  if (meshImportLibrary) {
    std::ifstream infile(filename.c_str(), std::ios::in);
    long beg = 0, end = 0;
    infile.seekg(0, std::ios::beg);
    beg = (long)infile.tellg();
    infile.seekg(0, std::ios::end);
    end = (long)infile.tellg();
    infile.seekg(0, std::ios::beg);
    long fileSize = (end - beg);

    std::string buffer(std::istreambuf_iterator<char>(infile),
                  (std::istreambuf_iterator<char>()));
    infile.close();

    msc = meshImportLibrary->createMeshSystemContainer(
        filename.c_str(), buffer.c_str(), buffer.length(), 0);
    if (msc) {
      NVSHARE::MeshSystem *ms = meshImportLibrary->getMeshSystem(msc);
      min = glm::vec3(ms->mAABB.mMin[0], ms->mAABB.mMin[1], ms->mAABB.mMin[2]);
      max = glm::vec3(ms->mAABB.mMax[0], ms->mAABB.mMax[1], ms->mAABB.mMax[2]);

      float dy = fabs(max.y - min.y);
      float dz = fabs(max.z - min.z);
      bool bYup = (dy > dz);
      if (!bYup) {
        float tmp = min.y;
        min.y = min.z;
        min.z = -tmp;

        tmp = max.y;
        max.y = max.z;
        max.z = -tmp;
      }
      /*
      //this should work but there are some problems with the MeshImport library
      int totalMat = ms->mMaterialCount;
      for(size_t i=0;i<totalMat;i++) {
              NVSHARE::MeshMaterial pMat = ms->mMaterials[i];
              std::string name = pMat.mName;
              std::string metadata = pMat.mMetaData;
              int startGarbage = metadata.find_first_of("%20");
              int endGarbage = metadata.find_last_of("%20");
              if(endGarbage!= std::string::npos)
                      metadata = metadata.erase(endGarbage-2, 3);
              if(startGarbage!= std::string::npos)
                      metadata = metadata.erase(startGarbage, 3);

              int diffuseIndex = metadata.find("diffuse=");
              if( diffuseIndex != std::string::npos)
                      metadata = metadata.erase(diffuseIndex,
      strlen("diffuse=")); int len = metadata.length(); if(len>0) { string
      fullName="";
                      //get filename only
                      int index = metadata.find_last_of("\\");

                      if(index == string::npos) {
                              fullName.append(metadata);
                      } else {
                              std::string fileNameOnly =
      metadata.substr(index+1, metadata.length());
                              fullName.append(fileNameOnly);
                      }
                      //see if the material exists already?
                      bool exists = true;
                      if(materialNames.find(name)==materialNames.end() )
                               exists = false;

                      if(!exists)
                              materialNames[name] = (fullName);
              }
      }*/

      int totalVertices = 0, lastSubMeshIndex = 0;
      for (size_t i = 0; i < ms->mMeshCount; i++) {
        NVSHARE::Mesh *pMesh = ms->mMeshes[i];

        for (size_t j = 0; j < pMesh->mVertexCount; j++) {
          Vertex v;
          v.pos.x = pMesh->mVertices[j].mPos[0];
          if (bYup) {
            v.pos.y = pMesh->mVertices[j].mPos[1];
            v.pos.z = pMesh->mVertices[j].mPos[2];
          } else {
            v.pos.y = pMesh->mVertices[j].mPos[2];
            v.pos.z = -pMesh->mVertices[j].mPos[1];
          }

          v.normal.x = pMesh->mVertices[j].mNormal[0];
          if (bYup) {
            v.normal.y = pMesh->mVertices[j].mNormal[1];
            v.normal.z = pMesh->mVertices[j].mNormal[2];
          } else {
            v.normal.y = pMesh->mVertices[j].mNormal[2];
            v.normal.z = -pMesh->mVertices[j].mNormal[1];
          }
          v.uv.x = pMesh->mVertices[j].mTexel1[0];
          v.uv.y = pMesh->mVertices[j].mTexel1[1];
          vertices.push_back(v);
        }

        for (size_t j = 0; j < pMesh->mSubMeshCount; j++) {
          SubMesh s;
          NVSHARE::SubMesh *pSubMesh = pMesh->mSubMeshes[j];
          // copy to container
          s.materialName = pSubMesh->mMaterialName;

          s.indices.resize(pSubMesh->mTriCount * 3);
          memcpy(&(s.indices[0]), pSubMesh->mIndices,
                 sizeof(unsigned int) * pSubMesh->mTriCount * 3);
          submeshes.push_back(s);
        }

        // add the total number of vertices to offset the mesh indices
        if (totalVertices != 0) {
          for (size_t m = lastSubMeshIndex; m < submeshes.size(); m++) {
            for (size_t n = 0; n < submeshes[m].indices.size(); n++) {
              submeshes[m].indices[n] += totalVertices;
            }
          }
        }
        totalVertices += pMesh->mVertexCount;
        lastSubMeshIndex = submeshes.size();
      }
    }
  }

  return true;
}
