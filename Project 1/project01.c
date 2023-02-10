#include "project01.h"

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

char * BakingInputHardcodeTest[] = {
    "Bake-Bagel",
    "No-Request",
    "No-Request",
    "Bake-Bagel",
    "Bake-Bagel",
    "Bake-Bagel",
    "Bake-Bagel",
    "Bake-Bagel",
    "Bake-Bagel",
    "Bake-Bagel",
    "Bake-Bagel",
    "Bake-Bagel",
    "No-Request",
    "Bake-Bagel",
    "No-Request",
    "Bake-Bagel",
    "No-Request",
    "Bake-Bagel",
    "Bake-Bagel",
    "Bake-Bagel",
    "Bake-Bagel",
    "Bake-Bagel",
    "No-Request",
    "Bake-Bagel",
    "Bake-Bagel",
    "Bake-Bagel",
    "Bake-Bagel",
    "Bake-Bagel",
    "No-Request",
    "Bake-Bagel",
    "No-Request",
    "Bake-Bagel",
    "No-Request",
    "Bake-Bagel",
    "No-Request",
    "Bake-Bagel"
};


void process_pipeline(char ** );
int checkInputStage(char ** );
long int count = 0;
//#define BUFSIZE=10 

void main(int argc, char * argv[]) {


    //run pipeline with hardcoded inputs
    //count=36;
    //process_pipeline(BakingInputHardcodeTest);


    //From file
    char * input[1000005]; //= {"0"};

    FILE * fp = fopen(argv[1], "r");
    //FILE * fp = fopen("trace2_", "r");
    int inputlength = 15;
    char buff[inputlength];

    char inputtext[11];
    while (fgets(buff, inputlength, fp) != NULL) {
        strncpy(inputtext, buff, 10);
        inputtext[11] = '\0';
        input[count] = malloc(inputlength);
        if (strcmp(inputtext, "No-Request") == 0) {
            input[count] = "No-Request";
        } else if (strcmp(inputtext, "Bake-Bagel") == 0) {
            input[count] = "Bake-Bagel";

        } else {
            input[count] = "Bake-Baguette";

        }


        // strcpy(input[count], buff);

        //printf("%ld Read Value %s\n",count,input[count]);
        count++;
    }
    fclose(fp);
    /*
    for(int j=0;j<100;j++){
    //strncpy(inputtext,input[j],11);
      //  inputtext[13]='\0';
        printf("%s input value- %d-- %d -- %d\n", input[j], strcmp(input[j],"Bake-Bagel"), strcmp(input[j],"Bake-Baguette"),input[j]=="Bake-Baguette"  );
       
    }
    */
    // run pipeline with file inputs

    process_pipeline(input);

}

void init_pipepine(char ** stages) {
    for (int c = 0; c < 12; c++) {
        stages[c] = "-";
    }
}

void process_pipeline(char ** bakinginput) {

    //char *OutputString  = "";

    char * BakingStagesNames[] = {
        "scaling",
        "mixing",
        "fermentation",
        "folding",
        "dividing",
        "rounding",
        "resting",
        "shaping",
        "proofing",
        "baking",
        "cooling",
        "stocking"
    };

    long cycles = 0;
    long bagel_baked = 0, baguette_baked = 0, no_request = 0;
    //long i = 0;
    int loop = 1;
    char * inputValue = "";
    char * BakingStages[12];

    init_pipepine(BakingStages);
    //printf("count value %ld\n", count);

    while (loop == 1) {


        // shift elements to next stage 
        if (cycles > 0) {

            for (int b = 11; b > 0; b--) {
                BakingStages[b] = BakingStages[b - 1];

            }
        }
        //if (i <= BakingInput.Length - 1)

        if (cycles < count) {
            inputValue = bakinginput[cycles];
            //printf("String compare %d -- %s",inputValue == "No-Request", inputValue);
            if (inputValue == "Bake-Bagel")
                bagel_baked++;
            else if (inputValue == "Bake-Baguette")
                baguette_baked++;
            else if (inputValue == "No-Request")
                no_request++;
        } else
            inputValue = "-";

        BakingStages[0] = inputValue;




        if (cycles > 0) {

            // halt baking stage after 1000 bakings
            if (cycles % 1000 == 0)
                cycles = cycles + 10;
            // halt baking stage for 1 cycle
            else if ((bagel_baked + baguette_baked) % 10 == 0)
                cycles = cycles + 1;

            // halt baking stage for 2 cycles
            else if (BakingStages[9] == "Bake-Baguette")
                cycles = cycles + 2;

        }

        /*
        //Printing Log
        printf("Current Time: %ld\n", cycles);
        for (int b = 11; b >= 0; b--) {
            //if (strcmp(BakingStages[b], "") != 0)
            if (BakingStages[b] != "-") {
                //if (b == 0 || strcmp(BakingStages[b], "No-Request") == 0)
                if (b == 0 || BakingStages[b] != "No-Request") {
                    printf("[%d] %s [%s]\n", b + 1, BakingStagesNames[b], BakingStages[b]);
                }
            }
        }

        printf("\n\n");
        */
        cycles++;
        loop = checkInputStage(BakingStages);

    };
    printf("Baking count: %ld\n", bagel_baked + baguette_baked);
    printf(" -Bagel baked: %ld\n", bagel_baked);
    printf(" -Baguette baked: %ld\n", baguette_baked);
    printf("No request: %ld\n", no_request);
    printf("\nHow many minutes to bake: %ld\n", cycles);
    printf("\nPerformance (bakes/minutes): %.2f\n", (bagel_baked + baguette_baked) / (cycles * 1.00));
    //return NULL;
}

int checkInputStage(char ** bakingInputValues) {
    for (int i = 0; i < 12; i++) {
        //if (strcmp(bakingInputValues[i], "") != 0)
        if (bakingInputValues[i] != "-")
            return 1;
    }
    return 0;
}
