/**
 * Extreme Edge Cases
 * CS 241 - Fall 2019
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

int equals(char** first, char** second) {
    while (1) {
        if (*first == NULL && *second == NULL) {
            return 1;
        } else if (*first == NULL || *second == NULL) {
            return 0;
        }
        // printf("%s, %s\n", *first, *second);

        if (strcmp(*first, *second) != 0) {
            return 0;
        }
        first++;
        second++;
    }
    return 1;
}

int test_0(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    char* expected[10] = {
        "helloBaby",
        "iAmSexy",
        "owieyfasdlf5543038409348",
        "",
        "",
        "",
        "",
        "3thisIsATescta",
        "er",
        NULL
    };
    char** output = camelCaser("Hello baby. I am sexy. owieyfasdlf5543038409348__#@$3this is A TesCta.er.aasdfasfgvb");
    if (!equals(output, expected)) {
        destroy(output);
        return 0;
    } else {
        destroy(output);
        return 1;
    }
}

int test_1(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    char* expected[7] = {
        "lickMyNeck",
        "myBack",
        "myPu",
        "",
        "yAndMyCrack",
        "ahe",
        NULL
    };
    char** output = camelCaser("Lick my neck. My back. My pu$$y and my crack. ahe;naeofhaefn9830r1h19p813hdb ipu9cahweb9p7");
    
    if (!equals(output, expected)) {
        destroy(output);
        return 0;
    } else {
        destroy(output);
        return 1;
    }
}

int test_2(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    char* expected[19] = {
        "lFuck",
        "blaclhoeand",
        "",
        "",
        "somethin",
        "g",
        "",
        "",
        "8888J",
        "j",
        "",
        "",
        "",
        "fealnelmao",
        "",
        "",
        "",
        "142983Ncfaoen",
        NULL
    };

    char** output = camelCaser("L\tfuck\n.blaclhoeand  @@@somethin[g][]8888   J?J????fealnelmao%%%%14 2983ncfaoen\n!");
    
    if (!equals(output, expected)) {
        destroy(output);
        return 0;
    } else {
        destroy(output);
        return 1;
    }
}

int test_3(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    char* expected[52] = {
        "",
        "alne",
        "",
        "n",
        "lk",
        "",
        "",
        "",
        "",
        "",
        "",
        "uob",
        "ojn8183",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "aana",
        "",
        "b",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "mne",
        "",
        "",
        "",
        "",
        "hello",
        "",
        "",
        "",
        "n12",
        "",
        "",
        "",
        "ajne",
        "",
        NULL
    };

    char** output = camelCaser("'alne~`n%lk ` \n *)&*#@ UOB%oJN8183^# *&(_+? ;;aana::b !@#$%%^&*()_mne {}{[[hello]|| +n12+__=ajne-'");
    
    if (!equals(output, expected)) {
        destroy(output);
        return 0;
    } else {
        destroy(output);
        return 1;
    }
}

int test_4(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    char* expected[24] = {
        "",
        "aq",
        "aljnEa",
        "d",
        "s",
        "anei",
        "aN",
        "",
        "na",
        "eoaind",
        "",
        "",
        "q",
        "",
        "",
        "kejaJncz",
        "somethingLikeThis",
        "",
        "",
        "jjjadsd",
        "",
        "",
        "",
        NULL
    };
    char** output = camelCaser("%aq%aljn ea%d%s\n%anei;a\tn|| na;eoaind @!`q [][ keja jncz. something like this? <>jjjadsd!?-=972");
    if (!equals(output, expected)) {
        destroy(output);
        return 0;
    } else {
        destroy(output);
        return 1;
    }
}

int test_5(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    char* expected[24] = {
        "extraPolarIce",
        "",
        "",
        "",
        "grainger",
        "canSuckMyAss",
        "",
        "",
        "",
        "uwu",
        "aelnaifePolnareff",
        "",
        "",
        "",
        "",
        NULL
    };
    char** output = camelCaser("extra POLAR iCe!!! <grainger> can suck my ass... (uwu) aelnaife polnareff%%%.?1nones\n");
    if (!equals(output, expected)) {
        destroy(output);
        return 0;
    } else {
        destroy(output);
        return 1;
    }
}

int test_6(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    char* expected[3] = {
        "",
        "fuckShitBitchYoungShackWesIsReallyReallyRich",
	    NULL
    };
    char** output = camelCaser("*fuck shit bitch young shack wes is really really rich* mo bamba \t bingo");
    // printf("OVER HERE BITCH");
    // printf("%s", *output);
    if (!equals(output, expected)) {
        destroy(output);
        return 0;
    } else {
        destroy(output);
        return 1;
    }
}

int test_7(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    char* expected[4] = {
        "thisIsTheEndOfWakanda",
        "avengersAssemble",
        "iAmInevitable",
	    NULL
    };
    char** output = camelCaser("this is the end of wakanda. avengers ASSemble! I am inevitable - THANASS");
    if (!equals(output, expected)) {
        destroy(output);
        return 0;
    } else {
        destroy(output);
        return 1;
    }
}

int test_8(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    char* expected[4] = {
        "colorsWeaveIntoASpireOfFlame",
        "distantSparksCallToAPastStillUnnamed",
        "",
        NULL
    };
    char** output = camelCaser("colors weave into a spire of flame ! distant sparks call to a past still unnamed . +");
    if (!equals(output, expected)) {
        destroy(output);
        return 0;
    } else {
        destroy(output);
        return 1;
    }
}

int test_9(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    char* expected[28] = {
        "",
        "",
        "",
        "qazxsw2",
        "",
        "edcVfr",
        "5tgbNhy",
        "67ujm",
        "ki",
        "9ol",
        "",
        "",
        "p",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        NULL
    };
    char** output = camelCaser("`~!QAZxsw2@#EDC VFR$ 5TGB NHY^ 67ujm <KI* 9ol., )P:?/'[--+_+}{?>:");
    if (!equals(output, expected)) {
        destroy(output);
        return 0;
    } else {
        destroy(output);
        return 1;
    }
}

int test_10(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    char* expected[25] = {
        "aaa8888Hello",
	    NULL
    };
    char** output = camelCaser("aaa 8888hello.");
    if (!equals(output, expected)) {
        destroy(output);
        return 0;
    } else {
        destroy(output);
        return 1;
    }
}

int test_11(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    char* expected[5] = {
        "",
        "",
        "",
        "",
        NULL
    };
    char** output = camelCaser("\n.\f.\v.\t.\r");
    if (!equals(output, expected)) {
        destroy(output);
        return 0;
    } else {
        destroy(output);
        return 1;
    }
}

int test_12(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    char* expected[1] = {
        NULL
    };
    char** output = camelCaser("");
    if (!output) return 1;
    if (!equals(output, expected)) {
        destroy(output);
        return 0;
    } else {
        destroy(output);
        return 1;
    }
}

/*
 * Testing function for various implementations of camelCaser.
 *
 * @param  camelCaser   A pointer to the target camelCaser function.
 * @param  destroy      A pointer to the function that destroys camelCaser
 * 			output.
 * @return              Correctness of the program (0 for wrong, 1 for correct).
 */
int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    // TODO: Return 1 if the passed in function works properly; 0 if it doesn't.

    if (!test_0(camelCaser, destroy)) return 0;
    if (!test_1(camelCaser, destroy)) return 0;
    if (!test_2(camelCaser, destroy)) return 0;
    if (!test_3(camelCaser, destroy)) return 0;
    if (!test_4(camelCaser, destroy)) return 0;
    if (!test_5(camelCaser, destroy)) return 0;
    if (!test_6(camelCaser, destroy)) return 0; // fix this
    if (!test_7(camelCaser, destroy)) return 0; // fix this too
    if (!test_8(camelCaser, destroy)) return 0; // and this one
    if (!test_9(camelCaser, destroy)) return 0;
    if (!test_10(camelCaser, destroy)) return 0;
    if (!test_11(camelCaser, destroy)) return 0;
    // if (!test_12(camelCaser, destroy)) return 0;

    return 1;
}



