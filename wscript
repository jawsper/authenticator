
#
# This file is the default set of rules to compile a Pebble project.
#
# Feel free to customize this to your needs.
#

import os
import base64

top = '.'
out = 'build'

def options(ctx):
    ctx.load('pebble_sdk')

def configure(ctx):
    ctx.load('pebble_sdk')

def build(ctx):
    ctx.load('pebble_sdk')

    ctx.pbl_program(source=ctx.path.ant_glob('src/**/*.c'),
                    target='pebble-app.elf')

    js = file("src/js/pebble-js-app.js", "r").read()
    html = file("src/js/configuration.html", "r").read()
    try:
        os.mkdir('generated')
    except OSError: pass
    jsoutname = "generated/pebble-js-app.js"
    jsout = file(jsoutname, "w")
    jsout.write(js)

    jsonIdx = html.index("__JSON__")
    pre = html[:jsonIdx]
    post = html[jsonIdx+8:]

    pre += ' ' * ((3 - len(pre) % 3) % 3)
    jsout.write('var htmlpre="%s";\n' % base64.b64encode(pre))

    post += '<!--' + ' ' * ((3 - (len(post) + 1) % 3) % 3)
    jsout.write('var htmlpost="%shtml";\n' % base64.b64encode(post))

    jsout.close()

    ctx.pbl_bundle(elf='pebble-app.elf',
                   js=[jsoutname])
