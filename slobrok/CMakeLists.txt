# Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
vespa_define_module(
    DEPENDS
    fastos
    vespalib
    fnet
    configdefinitions
    staging_vespalib

    LIBS
    src/vespa/slobrok
    src/vespa/slobrok/server

    APPS
    src/apps/check_slobrok
    src/apps/sbcmd
    src/apps/slobrok

    TESTS
    src/tests/backoff
    src/tests/configure
    src/tests/mirrorapi
    src/tests/registerapi
    src/tests/standalone
    src/tests/startsome
    src/tests/startup
)
