## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):

    module = bld.create_ns3_module('ns3-dtn-bit', ['bin', 'utility'])
    module.source = [
        'module/dtn_package.cc'
        ]

    module_test = bld.create_ns3_module_test_library('ns3-dtn-bit')
    module_test.source = [
        'test/spectrum-interference-test.cc',
        'test/spectrum-value-test.cc',
        'test/spectrum-ideal-phy-test.cc',
        'test/spectrum-waveform-generator-test.cc',
        'test/tv-helper-distribution-test.cc',
        'test/tv-spectrum-transmitter-test.cc',
        ]
    
    headers = bld(features='ns3header')
    headers.module = 'spectrum'
    headers.source = [
        'model/spectrum-model.h',
        'model/spectrum-value.h',
        'model/spectrum-converter.h',
        'model/spectrum-signal-parameters.h',
        'model/spectrum-propagation-loss-model.h',
        'model/friis-spectrum-propagation-loss.h',
        'model/constant-spectrum-propagation-loss.h',
        'model/spectrum-phy.h',
        'model/spectrum-channel.h',
        'model/single-model-spectrum-channel.h', 
        'model/multi-model-spectrum-channel.h',
        'model/spectrum-interference.h',
        'model/spectrum-error-model.h',
        'model/spectrum-model-ism2400MHz-res1MHz.h',
        'model/spectrum-model-300kHz-300GHz-log.h',
        'model/wifi-spectrum-value-helper.h',
        'model/waveform-generator.h',       
        'model/spectrum-analyzer.h',
        'model/aloha-noack-mac-header.h',
        'model/aloha-noack-net-device.h',
        'model/half-duplex-ideal-phy.h',
        'model/half-duplex-ideal-phy-signal-parameters.h',
        'model/non-communicating-net-device.h',
        'model/microwave-oven-spectrum-value-helper.h',
        'model/tv-spectrum-transmitter.h',
        'helper/spectrum-helper.h',
        'helper/adhoc-aloha-noack-ideal-phy-helper.h',
        'helper/waveform-generator-helper.h',
        'helper/spectrum-analyzer-helper.h',
        'helper/tv-spectrum-transmitter-helper.h',
        'test/spectrum-test.h',
        ]

    if (bld.env['ENABLE_EXAMPLES']):
        bld.recurse('examples')


    bld.ns3_python_bindings()
