/************************************************************************* 
 * Copyright (C) 2017-2019 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of NORNS, a service that allows other programs to   *
 * start, track and manage asynchronous transfers of data resources      *
 * between different storage backends.                                   *
 *                                                                       *
 * See AUTHORS file in the top level directory for information regarding *
 * developers and contributors.                                          *
 *                                                                       *
 * This software was developed as part of the EC H2020 funded project    *
 * NEXTGenIO (Project ID: 671951).                                       *
 *     www.nextgenio.eu                                                  *
 *                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining *
 * a copy of this software and associated documentation files (the       *
 * "Software"), to deal in the Software without restriction, including   *
 * without limitation the rights to use, copy, modify, merge, publish,   *
 * distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to *
 * the following conditions:                                             *
 *                                                                       *
 * The above copyright notice and this permission notice shall be        *
 * included in all copies or substantial portions of the Software.       *
 *                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                 *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS   *
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN    *
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN     *
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE      *
 * SOFTWARE.                                                             *
 *************************************************************************/

#include "catch.hpp"
#include "utils.hpp"

SCENARIO("path normalization", "[utils::path_normalize]") {

//XXX
    GIVEN("a file path with redundant '/'") {
        WHEN("root slash is redundant") {
            THEN("the file path is returned with any redundant '/' stripped") {
                REQUIRE(norns::utils::lexical_normalize("/////file0").string() == "/file0");
            }
        }

        WHEN("root slash is redundant") {
            THEN("the file path is returned with any redundant '/' stripped") {
                REQUIRE(norns::utils::lexical_normalize("/////a/b/c/file0").string() == "/a/b/c/file0");
            }
        }

        WHEN("middle slash is redundant") {
            THEN("the file path is returned with any redundant '/' stripped") {
                REQUIRE(norns::utils::lexical_normalize("/a///b//////c/file0").string() == "/a/b/c/file0");
            }
        }

        WHEN("everything is redundant") {
            THEN("the file path is returned with any redundant '/' stripped") {
                REQUIRE(norns::utils::lexical_normalize("//a///b//////c///file0").string() == "/a/b/c/file0");
            }
        }

    }

//XXX
    GIVEN("a dir path with redundant '/'") {
        WHEN("root slash is redundant") {
            THEN("the dir path is returned with any redundant '/' stripped") {
                REQUIRE(norns::utils::lexical_normalize("/////subdir0", true).string() == "/subdir0/");
            }
        }

        WHEN("middle slash is redundant") {
            THEN("the dir path is returned with any redundant '/' stripped") {
                REQUIRE(norns::utils::lexical_normalize("/subdir0/////subdir1", true).string() == "/subdir0/subdir1/");
            }
        }

        WHEN("trailing slash is redundant") {
            THEN("the dir path is returned with any redundant '/' stripped") {
                REQUIRE(norns::utils::lexical_normalize("/subdir0/////", true).string() == "/subdir0/");
            }
        }

        WHEN("everything is redundant") {
            THEN("the dir path is returned with any redundant '/' stripped") {
                REQUIRE(norns::utils::lexical_normalize("//a///subdir0//b/c///subdir1///", true).string() == "/a/subdir0/b/c/subdir1/");
            }
        }
    }

    GIVEN("a file path that begins with '/' and has no '.' or '..' components") {
        WHEN("file path is empty") {
            THEN("the same file path is returned") {
                REQUIRE(norns::utils::lexical_normalize("").string() == "");
            }
        }

        WHEN("file path is '/'") {
            THEN("the same file path is returned") {
                REQUIRE(norns::utils::lexical_normalize("/").string() == "/");
            }
        }

        WHEN("file is in the root directory") {
            THEN("the same file path is returned") {
                REQUIRE(norns::utils::lexical_normalize("/file0").string() == "/file0");
            }
        }

        WHEN("file is in a subdir") {
            THEN("the same file path is returned") {
                REQUIRE(norns::utils::lexical_normalize("/a/b/c/file0").string() == "/a/b/c/file0");
            }
        }

        WHEN("file path ends with '/', and the file is in the root directory") {
            THEN("the file path is stripped of the trailing '/'") {
                REQUIRE(norns::utils::lexical_normalize("/file0/").string() == "/file0");
            }
        }

        WHEN("file path ends with '/', and the file is in a subdir") {
            THEN("the file path is stripped of the trailing '/'") {
                REQUIRE(norns::utils::lexical_normalize("/a/b/c/file0/").string() == "/a/b/c/file0");
            }
        }
    }

//XXX
    GIVEN("a file path that does not begin with '/' and has no '.' or '..' components") {
        WHEN("file path does not begin with '/', has no '.' or '..' components and the file is in the root directory") {
            THEN("a leading '/' is added to the file path") {
                REQUIRE(norns::utils::lexical_normalize("file0").string() == "/file0");
            }
        }

        WHEN("file path does not begin with '/', has no '.' or '..' components and the file is in a subdir") {
            THEN("a leading '/' is added to the file path") {
                REQUIRE(norns::utils::lexical_normalize("a/b/c/file0").string() == "/a/b/c/file0");
            }
        }

        WHEN("file path does not begin with '/', has no '.' or '..' components, ends with '/', and the file is in the root directory") {
            THEN("a leading '/' is adding to the file path, and the trailing '/' is removed") {
                REQUIRE(norns::utils::lexical_normalize("file0/").string() == "/file0");
            }
        }

        WHEN("file path does not begin with '/', has no '.' or '..' components, ends with '/', and the file is in a subdir") {
            THEN("a leading '/' is adding to the file path, and the trailing '/' is removed") {
                REQUIRE(norns::utils::lexical_normalize("a/b/c/file0/").string() == "/a/b/c/file0");
            }
        }
    }

//XXX
    GIVEN("a file path that begins with '/' and has '.' or '..' components") {

        WHEN("file path is '/.'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize("/.").string() == "/");
            }
        }

        WHEN("file path is '/..'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize("/..").string() == "/");
            }
        }

        WHEN("file path is '/./././././'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize("/./././././").string() == "/");
            }
        }

        WHEN("file path is '/../../../../'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize("/../../../..").string() == "/");
            }
        }

        WHEN("file path has one '.' after root and file is in the root directory") {
            THEN("'./' is stripped") {
                REQUIRE(norns::utils::lexical_normalize("/./file0").string() == "/file0");
            }
        }

        WHEN("file path has N '.' after root and file is in the root directory") {
            THEN("all './' are stripped") {
                REQUIRE(norns::utils::lexical_normalize("/./././././././file0").string() == "/file0");
            }
        }

        WHEN("file path has one './' after the filename and file is in the root directory") {
            THEN("'./' is stripped") {
                REQUIRE(norns::utils::lexical_normalize("/file0/./").string() == "/file0");
            }
        }

        WHEN("file path has N '.' after the filename and file is in the root directory") {
            THEN("all './' are stripped") {
                REQUIRE(norns::utils::lexical_normalize("/file0/./././././././").string() == "/file0");
            }
        }

        WHEN("file path has one '../' after the filename and file is in the root directory") {
            THEN("'../' is stripped") {
                REQUIRE(norns::utils::lexical_normalize("/file0/../").string() == "/");
            }
        }

        WHEN("file path has N '..' after the filename and file is in the root directory") {
            THEN("all '../' are stripped") {
                REQUIRE(norns::utils::lexical_normalize("/file0/../../../../../../../").string() == "/");
            }
        }
    }

//XXX
    GIVEN("a file path that does not begin with '/' and has '.' or '..' components") {

        WHEN("file path is '.'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize(".").string() == "/");
            }
        }

        WHEN("file path is '..'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize("..").string() == "/");
            }
        }

        WHEN("file path is './././././'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize("./././././").string() == "/");
            }
        }

        WHEN("file path is '../../../../'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize("../../../..").string() == "/");
            }
        }

        WHEN("file path has one '.' before the filename") {
            THEN("'./' is stripped") {
                REQUIRE(norns::utils::lexical_normalize("./file0").string() == "/file0");
            }
        }

        WHEN("file path has N '.' before the filename") {
            THEN("all './' are stripped") {
                REQUIRE(norns::utils::lexical_normalize("./././././././file0").string() == "/file0");
            }
        }

        WHEN("file path has one './' after the filename and file is in the root directory") {
            THEN("'./' is stripped") {
                REQUIRE(norns::utils::lexical_normalize("file0/./").string() == "/file0");
            }
        }

        WHEN("file path has N '.' after the filename and file is in the root directory") {
            THEN("all './' are stripped") {
                REQUIRE(norns::utils::lexical_normalize("file0/./././././././").string() == "/file0");
            }
        }

        WHEN("file path has one '../' after the filename and file is in the root directory") {
            THEN("'../' is stripped") {
                REQUIRE(norns::utils::lexical_normalize("file0/../").string() == "/");
            }
        }

        WHEN("file path has N '..' after the filename and file is in the root directory") {
            THEN("all '../' are stripped") {
                REQUIRE(norns::utils::lexical_normalize("file0/../../../../../../../").string() == "/");
            }
        }
    }

    //////////////////////////////////////// directories ///////////////////////////////////////////////////////////////
//XXX
    GIVEN("a dir path that begins with '/' and has no '.' or '..' components") {
        WHEN("dir path is empty") {
            THEN("an empty path is returned") {
                REQUIRE(norns::utils::lexical_normalize("", true).string() == "");
            }
        }

        WHEN("dir path is '/'") {
            THEN("the root path is returned") {
                REQUIRE(norns::utils::lexical_normalize("/", true).string() == "/");
            }
        }

        WHEN("dir is in the root directory") {
            THEN("a trailing '/' is added to the dir path") {
                REQUIRE(norns::utils::lexical_normalize("/subdir0", true).string() == "/subdir0/");
            }
        }

        WHEN("dir is in a subdir") {
            THEN("a trailing '/' is added to the dir path") {
                REQUIRE(norns::utils::lexical_normalize("/a/b/c/subdir0", true).string() == "/a/b/c/subdir0/");
            }
        }

        WHEN("dir path ends with '/', and the dir is in the root directory") {
            THEN("the same dir path is returned") {
                REQUIRE(norns::utils::lexical_normalize("/subdir0/", true).string() == "/subdir0/");
            }
        }

        WHEN("dir path ends with '/', and the dir is in a subdir") {
            THEN("the same dir path is returned") {
                REQUIRE(norns::utils::lexical_normalize("/a/b/c/subdir0/", true).string() == "/a/b/c/subdir0/");
            }
        }
    }

//XXX
    GIVEN("a dir path that begins with '/' and has '.' or '..' components") {

        WHEN("dir path is '/.'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize("/.", true).string() == "/");
            }
        }

        WHEN("dir path is '/..'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize("/..", true).string() == "/");
            }
        }

        WHEN("dir path is '/./././././'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize("/./././././", true).string() == "/");
            }
        }

        WHEN("dir path is '/../../../../'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize("/../../../..", true).string() == "/");
            }
        }

        WHEN("dir path has one '.' after root and dir is in the root directory") {
            THEN("'./' is stripped") {
                REQUIRE(norns::utils::lexical_normalize("/./subdir0", true).string() == "/subdir0/");
            }
        }

        WHEN("dir path has N '.' after root and dir is in the root directory") {
            THEN("all './' are stripped") {
                REQUIRE(norns::utils::lexical_normalize("/./././././././subdir0", true).string() == "/subdir0/");
            }
        }

        WHEN("dir path has one './' after the dirname and dir is in the root directory") {
            THEN("'./' is stripped") {
                REQUIRE(norns::utils::lexical_normalize("/subdir0/./", true).string() == "/subdir0/");
            }
        }

        WHEN("dir path has N '.' after the dirname and dir is in the root directory") {
            THEN("all './' are stripped") {
                REQUIRE(norns::utils::lexical_normalize("/subdir0/./././././././", true).string() == "/subdir0/");
            }
        }

        WHEN("dir path has one '../' after the dirname and dir is in the root directory") {
            THEN("'../' is stripped") {
                REQUIRE(norns::utils::lexical_normalize("/subdir0/../", true).string() == "/");
            }
        }

        WHEN("dir path has N '..' after the dirname and dir is in the root directory") {
            THEN("all '../' are stripped") {
                REQUIRE(norns::utils::lexical_normalize("/subdir0/../../../../../../../", true).string() == "/");
            }
        }

        WHEN("dir path has one '../' after the dirname and dir is not in the root directory") {
            THEN("the path to the parent is returned") {
                REQUIRE(norns::utils::lexical_normalize("/a/b/subdir0/subdir1/../", true).string() == "/a/b/subdir0/");
            }
        }
    }

//XXX
    GIVEN("a dir path that does not begin with '/' and has no '.' or '..' components") {
        WHEN("dir path is empty") {
            THEN("an empty path is returned") {
                REQUIRE(norns::utils::lexical_normalize("", true).string() == "");
            }
        }

        WHEN("dir is in the root directory") {
            THEN("a trailing '/' is added to the dir path") {
                REQUIRE(norns::utils::lexical_normalize("subdir0", true).string() == "/subdir0/");
            }
        }

        WHEN("dir is in a subdir") {
            THEN("a trailing '/' is added to the dir path") {
                REQUIRE(norns::utils::lexical_normalize("a/b/c/subdir0", true).string() == "/a/b/c/subdir0/");
            }
        }

        WHEN("dir path ends with '/', and the dir is in the root directory") {
            THEN("the same dir path is returned") {
                REQUIRE(norns::utils::lexical_normalize("subdir0/", true).string() == "/subdir0/");
            }
        }

        WHEN("dir path ends with '/', and the dir is in a subdir") {
            THEN("the same dir path is returned") {
                REQUIRE(norns::utils::lexical_normalize("a/b/c/subdir0/", true).string() == "/a/b/c/subdir0/");
            }
        }
    }

//XXX
    GIVEN("a dir path that does not begin with '/' and has '.' or '..' components") {

        WHEN("dir path is '.'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize(".", true).string() == "/");
            }
        }

        WHEN("dir path is '..'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize("..", true).string() == "/");
            }
        }

        WHEN("dir path is './././././'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize("./././././", true).string() == "/");
            }
        }

        WHEN("dir path is '../../../../'") {
            THEN("root is returned") {
                REQUIRE(norns::utils::lexical_normalize("../../../..", true).string() == "/");
            }
        }

        WHEN("dir path has one '.' after root and dir is in the root directory") {
            THEN("'./' is stripped") {
                REQUIRE(norns::utils::lexical_normalize("./subdir0", true).string() == "/subdir0/");
            }
        }

        WHEN("dir path has N '.' after root and dir is in the root directory") {
            THEN("all './' are stripped") {
                REQUIRE(norns::utils::lexical_normalize("./././././././subdir0", true).string() == "/subdir0/");
            }
        }

        WHEN("dir path has one './' after the dirname and dir is in the root directory") {
            THEN("'./' is stripped") {
                REQUIRE(norns::utils::lexical_normalize("subdir0/./", true).string() == "/subdir0/");
            }
        }

        WHEN("dir path has N '.' after the dirname and dir is in the root directory") {
            THEN("all './' are stripped") {
                REQUIRE(norns::utils::lexical_normalize("subdir0/./././././././", true).string() == "/subdir0/");
            }
        }

        WHEN("dir path has one '../' after the dirname and dir is in the root directory") {
            THEN("'../' is stripped") {
                REQUIRE(norns::utils::lexical_normalize("subdir0/../", true).string() == "/");
            }
        }

        WHEN("dir path has N '..' after the dirname and dir is in the root directory") {
            THEN("all '../' are stripped") {
                REQUIRE(norns::utils::lexical_normalize("subdir0/../../../../../../../", true).string() == "/");
            }
        }

        WHEN("dir path has one '../' after the dirname and dir is not in the root directory") {
            THEN("the path to the parent is returned") {
                REQUIRE(norns::utils::lexical_normalize("a/b/subdir0/subdir1/../", true).string() == "/a/b/subdir0/");
            }
        }
    }

///XXX
    GIVEN("a complex path with '.' or '..' components") {
        WHEN("a relative file path leads into the root directory") {
            THEN("the path is normalized") {
                REQUIRE(norns::utils::lexical_normalize("../subdir0/..////a/b/../c/file0").string() == "/a/c/file0");
            }
        }

        WHEN("a relative dir path leads into the root directory") {
            THEN("the path is normalized") {
                REQUIRE(norns::utils::lexical_normalize("../subdir0/..////a/b/../c/subdir1", true).string() == "/a/c/subdir1/");
            }
        }

        WHEN("a relative file path leads into the root directory") {
            THEN("the path is normalized") {
                REQUIRE(norns::utils::lexical_normalize("../subdir0/..////a/b/../b/file0").string() == "/a/b/file0");
            }
        }

        WHEN("a relative dir path leads into the root directory") {
            THEN("the path is normalized") {
                REQUIRE(norns::utils::lexical_normalize("../subdir0/..////a/b/../b/subdir1", true).string() == "/a/b/subdir1/");
            }
        }

        WHEN("a relative file path leads out of the root directory") {
            THEN("the returned file path points to the root directory ") {
                REQUIRE(norns::utils::lexical_normalize("../subdir0/..////a/b/../c/../d/../../file0").string() == "/file0");
            }
        }

        WHEN("a relative dir path leads out of the root directory") {
            THEN("the returned dir path points to the root directory ") {
                REQUIRE(norns::utils::lexical_normalize("../subdir0/..////a/b/../c/../d/../../../../../a/../subdir0", true).string() == "/subdir0/");
            }
        }

    }


}
