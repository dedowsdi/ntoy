## ntoy
node toy for OSG, mostly used for shader edit.

## help
```
Usage: ntoy [options] [filename]

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


Options:
  --comp            Observe comp shader.
  --define          Add define to osg::StateSet. e.g. --define NAME --define
                    "NAME=X Y Z"
  --export-texture  Read in script, export textures. All other option ignored.
                    See example for detail.
  --frag            Observe frag shader.
  --geom            Observe geom shader.
  --geometry        create geometry with n vertices in LINES draw mode, read it
                    as node file
  --help-all        Display all command line, env vars and keyboard & mouse
                    bindings.
  --help-env        Display environmental variables available
  --help-keys       Display keyboard & mouse bindings available
  --shader          Observe shader.
  --shadertoy       Shader toy, ignore node file, draw unit ndc quad. Create
                    toy.frag If no --frag exists, it's content is
                    NTOY_DEFAULT_FRAG file or predefined. Don't use this option
                    with --geometry or positional node If you want to try node
                    with shaders, use "--frag fragName node.osgt" instead.
  --shape           create shape drawable with osg builtin shape, read it as
                    node file
  --tesc            Observe tesc shader.
  --tese            Observe tese shader.
  --texture1d       Load 1d texture, start from unit 0. You must specify name
                    min_filter mag_filter wrap_s. It's case insensitive. e.g.
                     --texture1d name linear linear repeat
  --texture2d       Load 2d texture, start from unit 0. You must specify name
                    min_filter mag_filter wrap_s wrap_t. It's case insensitive.
                    e.g.
                     --texture2d name linear linear repeat repeat
  --texture3d       Load 3d texture, start from unit 0. You must specify name
                    min_filter mag_filter wrap_s wrap_t wrap_r. It's case
                    insensitive. e.g.
                     --texture3d name linear linear repeat repeat repeat
  --vert            Observe vert shader.
  -h or --help      Display command line parameters
```
