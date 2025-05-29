from random import randint
def ecu_data() -> list:
    rpm_graph = randint(2, 3)
    rpm_bar = rpm_graph + 6
    vacuum = rpm_graph - 2
    mass_air = 6 + vacuum
    return [randint(128, 129),               # engine coolant temp
            randint(125, 128),               # air intake temp
            randint(40, 41),                 # air fuel ratio
            0x02,                            #unknown
            mass_air,                        # mass air flow
            0x00,                            # throttle position sensor
            0x00,                            # bar graph RPM byte
            rpm_bar,                         # bar graph RPM byte
            0x00,                            # fuel cut flag
            rpm_graph,                       # rate per minute (graph)
            0x00,                            # unknown
            vacuum,                          # vacuum amount hg
            0x00,                            # oxygen sensor flag
            0x00,                            # cylinder cam sensor flag
            0x00,                            # check engine light codes
            0x00,                            # check engine light codes
            randint(0, 1),                   # vehicle speed sensor
            int((not vacuum)),               # injector value
            (vacuum * 8),                    # injector duty
            (96 + round(6.66 * rpm_graph)),  # ignition degrees
            0x02,                            # unknown
            0x02,                            # unknown
            0x00,                            # fuel pressure flag
            0x00,                            # MIL flag
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x00,                            # FTL flag
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            0x02,                            # unknown
            ]