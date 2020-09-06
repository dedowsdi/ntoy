#include <iostream>
#include <sstream>

#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <NodeToy.h>
#include <ToyViewer.h>

namespace ntoy
{

class NodeToyEventHandler : public osgGA::GUIEventHandler
{
public:
    NodeToyEventHandler(NodeToy* ntoy);

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

private:
    int _frame = 0;
    NodeToy* _toy = 0;
};

NodeToyEventHandler::NodeToyEventHandler(NodeToy* ntoy) : _toy(ntoy) {}

bool NodeToyEventHandler::handle(
    const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    switch (ea.getEventType())
    {
        case osgGA::GUIEventAdapter::KEYDOWN:
            switch (ea.getKey())
            {
                case osgGA::GUIEventAdapter::KEY_F11:
                    _toy->reportBound();
                    break;

                case osgGA::GUIEventAdapter::KEY_F12:
                    _toy->saveNode();
                    break;

                case osgGA::GUIEventAdapter::KEY_A:
                    _toy->toggleAxes();
                    break;

                default:
                    break;
            }
            break;

        case osgGA::GUIEventAdapter::MOVE:
            _toy->updateMouse(osg::Vec2(ea.getX(), ea.getY()));
            break;

        case osgGA::GUIEventAdapter::RESIZE:
            _toy->updateResolution(osg::Vec2(ea.getWindowWidth(), ea.getWindowHeight()));
            break;

        case osgGA::GUIEventAdapter::FRAME:
            if (_toy->getExportTextures())
            {
                auto viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
                if (viewer && viewer->getFrameStamp()->getFrameNumber() >= 2)
                {
                    _toy->exportTextures();
                    _toy->getRoot()->setNodeMask(0);
                    viewer->getEventQueue()->quitApplication();
                }
            }
            break;
        default:
            break;
    }
    return false;  // return true will stop event
}

}  // namespace ntoy

int main(int argc, char* argv[])
{
    osg::ArgumentParser args(&argc, argv);

    auto usage = args.getApplicationUsage();

    auto desc = R"0(

  This small util is used to edit osg text node, shader in real time.

  Examples:

  ntoy --shadertoy --frag fun.frag

  Observing fun.frag, reload it if it's changed.

  ntoy mesh.osgt

  Observing mesh.osgt, reload it if it's changed.

  ntoy --vert x.vert --frag y.frag mesh.osgt

  Create program with x.vert and y.frag, use it to render mesh.osgt. Observing x.vert,
  y.frag, mesh.osgt, reload if changed.

  ntoy --export-texture script

  Export shader textures, each line of script is an export item:
     output_name frag s t pixel_format pixel_type packing

  pixel_format and pixel_type is literal enum without the GL_ prefix, it's case insensitive. Note it
  always overwrite existing file.

)0";
    std::stringstream ss;
    ss << args.getApplicationName() + " [options] [filename]" << desc;
    usage->setCommandLineUsage(ss.str());

    usage->addKeyboardMouseBinding("F12", "Save.");
    usage->addKeyboardMouseBinding("F11", "Output bounding.");
    usage->addKeyboardMouseBinding("a", "Toggle axes.");

    usage->addCommandLineOption("--export-texture",
        "Read in script, export textures. All other option ignored. "
        "See example for detail.");
    usage->addCommandLineOption("--vert", "Observe vert shader.");
    usage->addCommandLineOption("--geom", "Observe geom shader.");
    usage->addCommandLineOption("--frag", "Observe frag shader.");
    usage->addCommandLineOption("--tesc", "Observe tesc shader.");
    usage->addCommandLineOption("--tese", "Observe tese shader.");
    usage->addCommandLineOption("--comp", "Observe comp shader.");
    usage->addCommandLineOption("--shader", "Observe shader.");
    usage->addCommandLineOption("--define",
        "Add define to osg::StateSet. e.g. --define NAME --define \"NAME=X Y Z\"");
    usage->addCommandLineOption("--shadertoy",
        "Shader toy, ignore node file, draw unit ndc quad. Create toy.frag If no --frag "
        "exists, it's content is NTOY_DEFAULT_FRAG file or predefined. Don't use this "
        "option with --geometry or positional node If you want to "
        "try node with shaders, use \"--frag fragName node.osgt\" instead.");
    usage->addCommandLineOption("--texture1d",
        "Load 1d texture, start from unit 0. You must specify name "
        "min_filter mag_filter wrap_s. It's case insensitive. e.g.\n "
        "--texture1d name linear linear repeat");
    usage->addCommandLineOption("--texture2d",
        "Load 2d texture, start from unit 0. You must specify name "
        "min_filter mag_filter wrap_s wrap_t. It's case insensitive. e.g.\n "
        "--texture2d name linear linear repeat repeat");
    usage->addCommandLineOption("--texture3d",
        "Load 3d texture, start from unit 0. You must specify name "
        "min_filter mag_filter wrap_s wrap_t wrap_r. It's case insensitive. e.g.\n "
        "--texture3d name linear linear repeat repeat repeat");
    usage->addCommandLineOption("--geometry",
        "create geometry with n vertices in LINES draw mode, read it as node file");
    usage->addCommandLineOption("--shape",
        "create shape drawable with osg builtin shape, read it as node file");

    auto helpType = 0u;
    if ((helpType = args.readHelpType()))
    {
        usage->write(std::cout, helpType);
        return 0;
    }

    if (args.errors())
    {
        args.writeErrorMessages(std::cout);
        return 1;
    }

    if (args.argc() <= 1)
    {
        usage->write(std::cout, osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    toy::ToyViewer viewer;
    viewer.addEventHandler(new toy::ViewerDebugHandler(&viewer));

    viewer.realize();

    ntoy::NodeToy toy(args, &viewer);

    auto root = toy.getRoot();
    root->addEventCallback(new ntoy::NodeToyEventHandler(&toy));

    viewer.setSceneData(root);
    viewer.addEventHandler(new osgGA::StateSetManipulator(root->getOrCreateStateSet()));
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgViewer::HelpHandler(usage));
    viewer.addEventHandler(new osgViewer::ScreenCaptureHandler);

    args.reportRemainingOptionsAsUnrecognized();
    if (args.errors())
    {
        args.writeErrorMessages(std::cout);
    }

    return viewer.run();
}
