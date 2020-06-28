#include <NodeToy.h>

#include <algorithm>
#include <ctime>
#include <iomanip>

#include <osg/AutoTransform>
#include <osg/Group>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/io_utils>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgGA/OrbitManipulator>
#include <osgViewer/Viewer>
#include <osg/ShapeDrawable>

#include <cassert>
#include <OsgFactory.h>
#include <OsgQuery.h>
#include <Resource.h>
#include <StringUtil.h>

namespace ntoy
{

namespace
{

auto toyVertexSource = R"0( #version 120

void main( void )
{
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = gl_Vertex;
})0";

bool addResource(osg::ArgumentParser& parser, const std::string& option,
    ResourceObserver& observer, NodeToy& ntoy,
    void (NodeToy::*callback)(const std::string&))
{
    std::string file;
    if (parser.read(option, file))
    {
        observer.addResource(Resource(file,
            [&ntoy, callback](const std::string& fname) { (ntoy.*callback)(fname); }));
        return true;
    }
    else
    {
        return false;
    }
}

osg::Texture::FilterMode stringToFilterMode(const std::string& s);
std::string filterModeToString(osg::Texture::FilterMode mode);

osg::Texture::WrapMode stringToWrapMode(const std::string& s);
std::string wrapModeToString(osg::Texture::WrapMode mode);

GLenum stringToPixelFormat(const std::string& s);
GLenum stringToPixelType(const std::string& s);

osg::Texture::FilterMode stringToFilterMode(const std::string& s)
{
    auto us = sutil::toupper(s);

    if (us == "LINEAR")
        return osg::Texture::LINEAR;
    if (us == "LINEAR_MIPMAP_LINEAR")
        return osg::Texture::LINEAR_MIPMAP_LINEAR;
    if (us == "LINEAR_MIPMAP_NEAREST")
        return osg::Texture::LINEAR_MIPMAP_NEAREST;
    if (us == "NEAREST")
        return osg::Texture::NEAREST;
    if (us == "NEAREST_MIPMAP_LINEAR")
        return osg::Texture::NEAREST_MIPMAP_LINEAR;
    if (us == "NEAREST_MIPMAP_NEAREST")
        return osg::Texture::NEAREST_MIPMAP_NEAREST;

    throw std::runtime_error(s + " is not a valid FilterMode");
}

std::string filterModeToString(osg::Texture::FilterMode mode)
{
    switch (mode)
    {
        case osg::Texture::LINEAR:
            return "LINEAR";
        case osg::Texture::LINEAR_MIPMAP_LINEAR:
            return "LINEAR_MIPMAP_LINEAR";
        case osg::Texture::LINEAR_MIPMAP_NEAREST:
            return "LINEAR_MIPMAP_NEAREST";
        case osg::Texture::NEAREST:
            return "NEAREST";
        case osg::Texture::NEAREST_MIPMAP_LINEAR:
            return "NEAREST_MIPMAP_LINEAR";
        case osg::Texture::NEAREST_MIPMAP_NEAREST:
            return "NEAREST_MIPMAP_NEAREST";
    }
    throw std::runtime_error(" Unknown FilterMode " + std::to_string(mode));
}

osg::Texture::WrapMode stringToWrapMode(const std::string& s)
{
    auto us = sutil::toupper(s);

    if (us == "CLAMP")
        return osg::Texture::CLAMP;
    if (us == "CLAMP_TO_EDGE")
        return osg::Texture::CLAMP_TO_EDGE;
    if (us == "CLAMP_TO_BORDER")
        return osg::Texture::CLAMP_TO_BORDER;
    if (us == "REPEAT")
        return osg::Texture::REPEAT;
    if (us == "MIRROR")
        return osg::Texture::MIRROR;

    throw std::runtime_error(" Unknown WrapMode " + us);
}

std::string wrapModeToString(osg::Texture::WrapMode mode)
{
    switch (mode)
    {
        case osg::Texture::CLAMP:
            return "CLAMP";
        case osg::Texture::CLAMP_TO_EDGE:
            return "CLAMP_TO_EDGE";
        case osg::Texture::CLAMP_TO_BORDER:
            return "CLAMP_TO_BORDER";
        case osg::Texture::REPEAT:
            return "REPEAT";
        case osg::Texture::MIRROR:
            return "MIRROR";
    }

    throw std::runtime_error(" Unknown WrapMode " + std::to_string(mode));
}

GLenum stringToPixelFormat(const std::string& s)
{
    auto us = sutil::toupper(s);

    if (us == "RED")
        return GL_RED;
    if (us == "RG")
        return GL_RG;
    if (us == "RGB")
        return GL_RGB;
    if (us == "BGR")
        return GL_BGR;
    if (us == "RGBA")
        return GL_RGBA;
    if (us == "BGRA")
        return GL_BGRA;
    if (us == "RED_INTEGER")
        return GL_RED_INTEGER;
    if (us == "RG_INTEGER")
        return GL_RG_INTEGER;
    if (us == "RGB_INTEGER")
        return GL_RGB_INTEGER;
    if (us == "BGR_INTEGER")
        return GL_BGR_INTEGER;
    if (us == "RGBA_INTEGER")
        return GL_RGBA_INTEGER;
    if (us == "BGRA_INTEGER")
        return GL_BGRA_INTEGER;
    if (us == "STENCIL_INDEX")
        return GL_STENCIL_INDEX;
    if (us == "DEPTH_COMPONENT")
        return GL_DEPTH_COMPONENT;
    if (us == "DEPTH_STENCIL")
        return GL_DEPTH_STENCIL;

    throw std::runtime_error(" Unknown pixel format " + us);
}

GLenum stringToPixelType(const std::string& s)
{
    auto us = sutil::toupper(s);

    if (us == "UNSIGNED_BYTE")
        return GL_UNSIGNED_BYTE;
    if (us == "BYTE")
        return GL_BYTE;
    if (us == "UNSIGNED_SHORT")
        return GL_UNSIGNED_SHORT;
    if (us == "SHORT")
        return GL_SHORT;
    if (us == "UNSIGNED_INT")
        return GL_UNSIGNED_INT;
    if (us == "INT")
        return GL_INT;
    if (us == "HALF_FLOAT")
        return GL_HALF_FLOAT;
    if (us == "FLOAT")
        return GL_FLOAT;
    if (us == "UNSIGNED_BYTE_3_3_2")
        return GL_UNSIGNED_BYTE_3_3_2;
    if (us == "UNSIGNED_BYTE_2_3_3_REV")
        return GL_UNSIGNED_BYTE_2_3_3_REV;
    if (us == "UNSIGNED_SHORT_5_6_5")
        return GL_UNSIGNED_SHORT_5_6_5;
    if (us == "UNSIGNED_SHORT_5_6_5_REV")
        return GL_UNSIGNED_SHORT_5_6_5_REV;
    if (us == "UNSIGNED_SHORT_4_4_4_4")
        return GL_UNSIGNED_SHORT_4_4_4_4;
    if (us == "UNSIGNED_SHORT_4_4_4_4_REV")
        return GL_UNSIGNED_SHORT_4_4_4_4_REV;
    if (us == "UNSIGNED_SHORT_5_5_5_1")
        return GL_UNSIGNED_SHORT_5_5_5_1;
    if (us == "UNSIGNED_SHORT_1_5_5_5_REV")
        return GL_UNSIGNED_SHORT_1_5_5_5_REV;
    if (us == "UNSIGNED_INT_8_8_8_8")
        return GL_UNSIGNED_INT_8_8_8_8;
    if (us == "UNSIGNED_INT_8_8_8_8_REV")
        return GL_UNSIGNED_INT_8_8_8_8_REV;
    if (us == "UNSIGNED_INT_10_10_10_2")
        return GL_UNSIGNED_INT_10_10_10_2;
    if (us == "UNSIGNED_INT_2_10_10_10_REV")
        return GL_UNSIGNED_INT_2_10_10_10_REV;

    throw std::runtime_error(" Unknown pixel type " + us);
}

}  // namespace

NodeToy::NodeToy(osg::ArgumentParser& args, osgViewer::Viewer* viewer) : _viewer(viewer)
{
    createScene();

    // If exporting textures, create program, ignore all other options.
    std::string script;
    _exportTextures = args.read("--export-texture", script);
    if (_exportTextures)
    {
        readExportTextures(script);
        _viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
        return;
    }

    bool shadertoy = args.find("--shadertoy") != -1;

    if (shadertoy && args.find("--frag") == -1)
    {
        auto file = createDefaultFrag();
        auto fragShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, file);
        _observer->addResource(createShaderResource(fragShader));
    }

    bool needProgram = shadertoy;

    needProgram |= readShaders(args);

    if (needProgram)
    {
        setupProgram();
    }

    readTextures(args);

    readDefines(args);

    if (shadertoy)
    {
        createShadertoy();
    }
    else
    {
        readNode(args);
    }
}

void NodeToy::reloadNode(const std::string& file)
{
    _sceneRoot->removeChild(0, _sceneRoot->getNumChildren());

    _node = osgDB::readNodeFile(file);
    if (!_node)
    {
        OSG_WARN << "Failed to read node from " << file << std::endl;
        return;
    }

    _sceneRoot->addChild(_node);

    // zoom camera, always focus at origin.
    auto manipulator =
        dynamic_cast<osgGA::OrbitManipulator*>(_viewer->getCameraManipulator());
    if (manipulator)
    {
        auto bound = _sceneRoot->getBound();
        auto radius = bound.center().length() + bound.radius();
        auto dist = std::max<double>(radius, 1e-6);

        // If you reisze app window, GraphicsContext::resizedImplementation would resize
        // projection matrix, there will be a one time size jump.
        auto camera = _viewer->getCamera();
        double left, right, bottom, top, zNear, zFar;
        camera->getProjectionMatrixAsFrustum(left, right, bottom, top, zNear, zFar);

        // smaller angle need bigger distance
        auto width = right - left;
        auto height = top - bottom;
        auto halfViewAngle = atan2(std::min(width, height) * 0.5f, zNear);

        dist /= std::sin(halfViewAngle);

        manipulator->setCenter(osg::Vec3());
        manipulator->setDistance(dist);
    }
}

void NodeToy::reportBound()
{
    if (_node)
    {
        auto& bound = _node->getBound();
        OSG_NOTICE << "Bounding sphere center : " << bound.center() << std::endl;
        OSG_NOTICE << "Bounding sphere radius : " << bound.radius() << std::endl;
    }
}

void NodeToy::saveNode()
{
    if (_node)
    {
        auto base = osgDB::getNameLessAllExtensions(_nodeFile);
        auto ext = osgDB::getFileExtension(_nodeFile);
        std::stringstream ss;
        ss << base << "." << _index << "." << ext;
        auto name = ss.str();

        if (osgDB::writeNodeFile(*_node, name))
        {
            OSG_NOTICE << "Write to " << name << std::endl;
        }
        else
        {
            OSG_NOTICE << "Failed to write to " << name << std::endl;
        }

        ++_index;
    }
}

void NodeToy::toggleAxes()
{
    if (_axes)
    {
        _axes->setNodeMask(!_axes->getNodeMask());
    }
}

void NodeToy::exportTextures()
{
    for (auto& et: _exportTextureList)
    {
        OSG_NOTICE << "Writing " << et.img->s() << "x" << et.img->t() << " "
                   << et.output_name << " with frag " << et.frag << std::endl;
        osgDB::writeImageFile(*et.img, et.output_name);
    }
}

void NodeToy::updateMouse(const osg::Vec2& mouse)
{
    if (_mouseUniform)
    {
        _mouseUniform->set(mouse);
    }
}

void NodeToy::updateResolution(const osg::Vec2& resolution)
{
    if (_resolutionUniform)
    {
        _resolutionUniform->set(resolution);
    }
}

std::string NodeToy::createDefaultFrag()
{
    std::string file = "toy.frag";

    if (osgDB::fileExists(file))
    {
        return file;
    }

    // create toy.frag if it doesn't exist
    std::ofstream ofs(file);

    // try NEDITOR_DEFAULT_FRAG
    auto def = std::getenv("NTOY_DEFAULT_FRAG");
    if (def)
    {
        std::ifstream ifs(def);
        if (ifs)
        {
            ofs << ifs.rdbuf();
            return file;
        }
    }

    // use predfined frag
    ofs << R"0(#version 120

#define PI 3.1415926535897932384626433832795
#define PI_2 ( PI * 0.5 )
#define PI_4 ( PI * 0.25 )
#define PI_8 ( PI * 0.125 )

uniform int osg_FrameNumber;
uniform float osg_FrameTime;
uniform float osg_DeltaFrameTime;
uniform float osg_SimulationTime;
uniform float osg_DeltaSimulationTime;
uniform vec2 mouse;
uniform vec2 resolution;

void main( void )
{
    vec2 st = gl_FragCoord.xy / resolution;
    gl_FragColor = vec4(st, 0, 1);
})0";

    return file;
}

void NodeToy::createScene()
{
    _root = new osg::Group;
    _root->setName("Root");

    _sceneRoot = new osg::Group;
    _sceneRoot->setName("SceneRoot");
    _root->addChild(_sceneRoot);

    // add _axes
    _axes = new osg::AutoTransform;
    _axes->setName("Axes");
    _axes->setAutoScaleToScreen(true);
    osgf::addConstantComputeBoundingSphereCallback(
        *_axes, osg::BoundingSphere(osg::Vec3(), 0.00001f));
    _root->addChild(_axes);

    auto pixelSize = 64.f;
    auto axesGraph = osgf::createAxes(pixelSize);
    _axes->addChild(axesGraph);

    // hide axes by default
    toggleAxes();

    _observer = new ResourceObserver;
    _root->addUpdateCallback(_observer);
}

void NodeToy::readTextures(osg::ArgumentParser& args)
{
    auto sceneSS = _sceneRoot->getOrCreateStateSet();

    std::string textureFile;
    int textureUnit = 0;
    std::string minFilter;
    std::string magFilter;
    std::string wrapS;
    std::string wrapT;
    std::string wrapR;
    while (args.read("--texture3d", textureFile, minFilter, magFilter, wrapS, wrapT, wrapR))
    {
        auto img = osgDB::readImageFile(textureFile);
        if (!img)
        {
            OSG_WARN << "Failed to load " << img << std::endl;
            continue;
        }

        try
        {
            auto texture = new osg::Texture3D(img);
            texture->setFilter(osg::Texture::MIN_FILTER, stringToFilterMode(minFilter));
            texture->setFilter(osg::Texture::MAG_FILTER, stringToFilterMode(magFilter));
            texture->setWrap(osg::Texture::WRAP_S, stringToWrapMode(wrapS));
            texture->setWrap(osg::Texture::WRAP_T, stringToWrapMode(wrapT));
            texture->setWrap(osg::Texture::WRAP_R, stringToWrapMode(wrapR));

            sceneSS->setTextureAttributeAndModes(textureUnit++, texture);
        }
        catch (const std::runtime_error& e)
        {
            OSG_WARN << e.what() << std::endl;
        }
    }

    textureUnit = 0;
    while (args.read("--texture2d", textureFile, minFilter, magFilter, wrapS, wrapT))
    {
        auto img = osgDB::readImageFile(textureFile);
        if (!img)
        {
            OSG_WARN << "Failed to load " << img << std::endl;
            continue;
        }

        try
        {
            auto texture = new osg::Texture2D(img);
            texture->setFilter(osg::Texture::MIN_FILTER, stringToFilterMode(minFilter));
            texture->setFilter(osg::Texture::MAG_FILTER, stringToFilterMode(magFilter));
            texture->setWrap(osg::Texture::WRAP_S, stringToWrapMode(wrapS));
            texture->setWrap(osg::Texture::WRAP_T, stringToWrapMode(wrapT));

            sceneSS->setTextureAttributeAndModes(textureUnit++, texture);
        }
        catch (const std::runtime_error& e)
        {
            OSG_WARN << e.what() << std::endl;
        }
    }

    textureUnit = 0;
    while (args.read("--texture1d", textureFile, minFilter, magFilter, wrapS))
    {
        auto img = osgDB::readImageFile(textureFile);
        if (!img)
        {
            OSG_WARN << "Failed to load " << img << std::endl;
            continue;
        }

        try
        {
            auto texture = new osg::Texture1D(img);
            texture->setFilter(osg::Texture::MIN_FILTER, stringToFilterMode(minFilter));
            texture->setFilter(osg::Texture::MAG_FILTER, stringToFilterMode(magFilter));
            texture->setWrap(osg::Texture::WRAP_S, stringToWrapMode(wrapS));

            sceneSS->setTextureAttributeAndModes(textureUnit++, texture);
        }
        catch (const std::runtime_error& e)
        {
            OSG_WARN << e.what() << std::endl;
        }
    }
}

bool NodeToy::readShaders(osg::ArgumentParser& args)
{
    bool b = false;

    std::string file;
    while (args.read("--shader", file))
    {
        b |= readShader(args, "--shader") != 0;
    }

    b |= (_vert = readShader(args, "--vert")) != 0;
    b |= (_geom = readShader(args, "--geom")) != 0;
    b |= (_frag = readShader(args, "--frag")) != 0;
    b |= (_tesc = readShader(args, "--tesc")) != 0;
    b |= (_tese = readShader(args, "--tese")) != 0;
    b |= (_comp = readShader(args, "--comp")) != 0;

    return b;
}

osg::Shader* NodeToy::readShader(
    osg::ArgumentParser& args, const std::string& option, int shaderType)
{
    if (!_program)
    {
        _program = new osg::Program;
    }

    std::string file;
    if (args.read(option, file))
    {
        auto shader =
            osgDB::readShaderFile(static_cast<osg::Shader::Type>(shaderType), file);

        if (shader)
        {
            _observer->addResource(createShaderResource(shader));
            _program->addShader(shader);
            return shader;
        }
    }

    return 0;
}

void NodeToy::setupProgram()
{
    assert(_program);

    auto sceneSS = _sceneRoot->getOrCreateStateSet();
    sceneSS->setAttributeAndModes(_program);

    // extra uniforms
    _mouseUniform = new osg::Uniform("mouse", osg::Vec2());
    sceneSS->addUniform(_mouseUniform);

    auto rect = osgq::getWindowRect(*_viewer);
    _resolutionUniform = new osg::Uniform("resolution", osg::Vec2(rect.z(), rect.w()));
    sceneSS->addUniform(_resolutionUniform);

    OSG_NOTICE << "Use program for scene root." << std::endl;
}

void NodeToy::readDefines(osg::ArgumentParser& args)
{
    auto sceneSS = _sceneRoot->getOrCreateStateSet();
    std::string name;
    while (args.read("--define", name))
    {
        auto pos = name.find_first_of('=');
        if (pos != std::string::npos)
        {
            sceneSS->setDefine(name.substr(0, pos), name.substr(pos));
        }
        else
        {
            sceneSS->setDefine(name);
        }
    }
}

void NodeToy::createShadertoy()
{
    if (_nodeFile.empty())
    {
        _sceneRoot->addChild(osgf::getNdcQuad());
        _vert = new osg::Shader(osg::Shader::VERTEX, toyVertexSource);
        _program->addShader(_vert);
        OSG_NOTICE << "Draw unit ndc quad with pass through vertex shader." << std::endl;
    }
}

void NodeToy::readNode(osg::ArgumentParser& args)
{
    int n = 0;
    std::string shapeName;
    if (args.read("--geometry", n, _nodeFile))
    {
        if (osgDB::fileExists(_nodeFile))
        {
            OSG_WARN << _nodeFile << " exists" << std::endl;
            abort();
        }
        else
        {
            auto geom = new osg::Geometry;
            auto vertices = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
            vertices->resize(n, osg::Vec3());
            geom->setVertexArray(vertices);
            geom->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, n));
            osgDB::writeNodeFile(*geom, _nodeFile);
        }
    }
    else if (args.read("--shape", shapeName, _nodeFile))
    {
        if (osgDB::fileExists(_nodeFile))
        {
            OSG_WARN << _nodeFile << " exists, ignore --shape option" << std::endl;
            abort();
        }
        else
        {
            osg::Shape* shape = 0;
            shapeName = sutil::tolower(shapeName);
            if ( shapeName == "sphere" )
                shape = new osg::Sphere;
            else if ( shapeName == "box" )
                shape = new osg::Box;
            else if ( shapeName == "cone" )
                shape = new osg::Cone;
            else if ( shapeName == "cylinder" )
                shape = new osg::Cylinder;
            else if ( shapeName == "capsule" )
                shape = new osg::Capsule;
            else if ( shapeName == "infiniteplane" )
                shape = new osg::InfinitePlane;
            else if ( shapeName == "trianglemesh" )
                shape = new osg::TriangleMesh;
            else if ( shapeName == "heightfield" )
                shape = new osg::HeightField;
            else if ( shapeName == "compositeshape" )
                shape = new osg::CompositeShape;
            else
            {
                OSG_WARN << "Invalid shape " << shapeName << ", fall back to sphere.";
                shape = new osg::Sphere;
            }

            auto geom = new osg::ShapeDrawable;
            geom->setShape(shape);

            auto hints = new osg::TessellationHints;
            geom->setTessellationHints(hints);

            osgDB::writeNodeFile(*geom, _nodeFile);
        }
    }
    else
    {
        // get 1st positional option as node file. This must be called after other option
        // read their values.
        for (auto i = 1; i < args.argc(); ++i)
        {
            auto arg = args[i];
            if (!args.isOption(arg))
            {
                _nodeFile = arg;
                break;
            }
        }
    }

    if (!_nodeFile.empty())
    {
        _observer->addResource(
                Resource(_nodeFile, [this](const std::string& fname) { reloadNode(fname); }));
    }
}

void NodeToy::readExportTextures(const std::string& script)
{
    std::ifstream ifs(script);
    if (!ifs)
    {
        OSG_FATAL << "Failed to read " << script << std::endl;
        return;
    }

    std::string line;
    auto rect = osgq::getWindowRect(*_viewer);

    while (std::getline(ifs, line))
    {
        ExportTexture et;
        int width = rect.z();
        int height = rect.w();
        int packing;
        std::string format;
        std::string type;

        // line can be output_name frag [width height]
        std::stringstream ss(line);
        if (ss >> et.output_name && ss >> et.frag && ss >> width && ss >> height &&
                ss >> format && ss >> type && ss >> packing)
        {
            et.img = new osg::Image;
            et.img->allocateImage(width, height, 1, stringToPixelFormat(format),
                    stringToPixelType(type), packing);
            addExportTexture(et);
        }
        else
        {
            OSG_FATAL << "Failed to read export texture from \"" << line << "\""
            << std::endl;
        }
    }
}

void NodeToy::addExportTexture(ExportTexture& et)
{
    auto camera = osgf::createRttCamera(
            0, 0, et.img->s(), et.img->t(), osg::Camera::FRAME_BUFFER_OBJECT);

    camera->attach(osg::Camera::COLOR_BUFFER0, et.img);

    auto program = new osg::Program;
    program->addShader(new osg::Shader(osg::Shader::VERTEX, toyVertexSource));
    program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, et.frag));

    auto ss = camera->getOrCreateStateSet();
    ss->setAttributeAndModes(program);
    ss->addUniform(new osg::Uniform("resolution", osg::Vec2(et.img->s(), et.img->t())));

    camera->addChild(osgf::getNdcQuad());

    _sceneRoot->addChild(camera);

    _exportTextureList.push_back(et);
    OSG_NOTICE << "Create rtt camera for " << et.output_name << std::endl;
}

}  // namespace ntoy
