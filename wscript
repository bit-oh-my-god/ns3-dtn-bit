## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
##    [ $ the module would be depended when new module was built  ]
    module = bld.create_ns3_module('ns3-dtn-bit', ['', ''])
    module.source = [
        'module/dtn_package.cpp'
        ]

    module_test = bld.create_ns3_module_test_library('ns3-dtn-bit')
    module_test.source = [
        'test/test_example_01.cpp'
        ]
    
    headers = bld(features='ns3header')
    headers.module = 'ns3-dtn-bit'
    headers.source = [
        'model/dtn_package.h'
        ]

    if (bld.env['ENABLE_EXAMPLES']):
        bld.recurse('examples')


