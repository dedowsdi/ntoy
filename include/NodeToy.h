#ifndef NTOY_NODETOY_H
#define NTOY_NODETOY_H

#include <osg/Node>

namespace osg
{
class AutoTransform;
}

namespace osgViewer
{
class Viewer;
}  // namespace osgViewer

namespace ntoy
{

class ResourceObserver;

class NodeToy
{
public:
    NodeToy(osg::ArgumentParser& args, osgViewer::Viewer* viewer);

    void reloadNode(const std::string& file);

    void reportBound();

    void saveNode();

    void toggleAxes();

    void exportTextures();

    void updateMouse(const osg::Vec2& mouse);

    void updateResolution(const osg::Vec2& resolution);

    // Read NEDITOR_DEF_FRAG or hard coded one. return new frag file name.
    std::string createDefaultFrag();

    osgViewer::Viewer* getViewer() { return _viewer; }
    void setViewer(osgViewer::Viewer* v) { _viewer = v; }

    osg::Group* getRoot() { return _root; }

    osg::Group* getSceneRoot() { return _sceneRoot; }

    osg::AutoTransform* getAxes() { return _axes; }

    osg::Node* getNode() { return _node; }

    osg::Shader* getVert() { return _vert; }

    osg::Shader* getGeom() { return _geom; }

    osg::Shader* getFrag() { return _frag; }

    osg::Shader* getTesc() { return _tesc; }

    osg::Shader* getTese() { return _tese; }

    osg::Shader* getComp() { return _comp; }

    osg::Program* getProgram() { return _program; }

    const std::string& getNodeFile() const { return _nodeFile; }
    void setNodeFile(const std::string& v) { _nodeFile = v; }

    bool getExportTextures() const { return _exportTextures; }
    void setExportTextures(bool v) { _exportTextures = v; }

private:
    // _root
    //   _sceneRoot
    //   _axes
    void createScene();

    void readTextures(osg::ArgumentParser& args);

    bool readShaders(osg::ArgumentParser& args);

    osg::Shader* readShader(
        osg::ArgumentParser& args, const std::string& option, int shaderType = -1);

    void setupProgram();

    void readDefines(osg::ArgumentParser& args);

    void createShadertoyNode();

    void readNode(osg::ArgumentParser& args);

    void readExportTextures(const std::string& script);

    struct ExportTexture
    {
        std::string output_name;
        std::string frag;
        osg::ref_ptr<osg::Image> img;
    };

    void addExportTexture(ExportTexture& et);

    bool _exportTextures = false;
    int _index = 0;
    osgViewer::Viewer* _viewer = 0;
    osg::Group* _root = 0;
    osg::Group* _sceneRoot = 0;
    osg::AutoTransform* _axes = 0;
    osg::Node* _node = 0;

    osg::Shader* _vert = 0;
    osg::Shader* _geom = 0;
    osg::Shader* _frag = 0;
    osg::Shader* _tesc = 0;
    osg::Shader* _tese = 0;
    osg::Shader* _comp = 0;
    osg::Program* _program = 0;
    osg::Uniform* _mouseUniform = 0;
    osg::Uniform* _resolutionUniform = 0;

    ResourceObserver* _observer = 0;

    std::string _nodeFile;

    using ExportTextureList = std::vector<ExportTexture>;
    ExportTextureList _exportTextureList;
};

}  // namespace ntoy

#endif // NTOY_NODETOY_H
