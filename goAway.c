#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>

//nie liczyæ czasu stania w jendym miejscu kiedy ¿aba jest na aucie T i ma wtedy te¿ ¿aba nie traciæ ¿yæ
//zrozumieæ dok³adnie o co chodzi w gotoxy, setColor i hideCursor
//usun¹æ rzeczy z size
//usun¹æ rzeczy z taksówk¹ i spróbowaæ od pocz¹tku to zrobiæ

//array sizes (game field sizes) that are used in defining the arrays 
#define CARS 10
#define CARS_T 5
#define LANES 25
#define COLUMNS 35
#define OBSTACLES 20

struct gameParameters {
    //color values
    int colorFrog;
    int colorCarB;
    int colorCarG;
    int colorCarT;
    int colorField;
    int colorObstacle;

    //array sizes (game field sizes)
    int cars;
    int carsT;
    int lanes;
    int columns;

    //elements' symbols
    int frogSymbol;
    int carBSymbol;
    int carGSymbol;
    int carsTSymbol;
    int obstacleSymbol;
    int size;

    //move values
    int left;
    int right;
    int up;
    int down;
    int carMove;

    //game constants
    int standByTime;
    int distanceCarG;
    int distanceCarT;
    int storkSpeed;

    //border symbols
    int horizontalBorder; //205
    int verticalBorder;   //186
    int topRight;         //187
    int topLeft;          //201
    int downRight;        //188
    int downLeft;         //200
    int laneSymbol;       //45

} parameters;

struct game {
    int score;
    int time;
    int lifes;
    time_t lastMoveTime;
    int collisionOccured;
} game;

//"bad" cars - are colliding with frog
struct CarsB {
    int y; //h
    int lane; //v
    int speed;
    int color;
    int direction;
    time_t lastMoveTime;
    int move;
    int size;
    int speeding;
} carsB[CARS];

//"good" cars - slow down when nearby the frog
struct CarsG {
    int y; //h
    int lane; //v
    int speed;
    int color;
    int direction;
    int move;
    int distanceToFrog;
    time_t lastMoveTime;
    int size;
} carsG[CARS];

struct CarsT {
    int y; //h
    int lane; //v
    int speed;
    int color;
    int direction;
    int move;
    int distanceToTake;
    time_t lastMoveTime;
    int size;
} carsT[CARS_T];

struct Frog {
    int x; //h
    int y; //v
    int color;
    int frogOnCar;
} frog;

struct obstacle {
    int x;
    int y;
    int color;
} obstacles[OBSTACLES];


//reading specific parameters
//border values
void readBorderParameters(const char* filename, struct gameParameters* params) {
    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    char line[100];
    char constant[50];
    int value;

    while (fgets(line, sizeof(line), file)) { //while there is a next line in the file
        memset(constant, 0, sizeof(constant));

        if (sscanf(line, "%49[^=]=%d", constant, &value) == 2) {
            //reading the border values
            if (strcmp(constant, "HORIZONTAL_BORDER") == 0) params->horizontalBorder = value;
            else if (strcmp(constant, "VERTICAL_BORDER") == 0) params->verticalBorder = value;
            else if (strcmp(constant, "TOP_RIGHT") == 0) params->topRight = value;
            else if (strcmp(constant, "TOP_LEFT") == 0) params->topLeft = value;
            else if (strcmp(constant, "DOWN_RIGHT") == 0) params->downRight = value;
            else if (strcmp(constant, "DOWN_LEFT") == 0) params->downLeft = value;
            else if (strcmp(constant, "LANE_SYMBOL") == 0) params->laneSymbol = value;
        }
    }
}

//moves values
void readMovesParameters(const char* filename, struct gameParameters* params) {
    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    char line[100];
    char constant[50];
    int value;

    while (fgets(line, sizeof(line), file)) {
        memset(constant, 0, sizeof(constant));

        if (sscanf(line, "%49[^=]=%d", constant, &value) == 2) {
            //reading move values
            if (strcmp(constant, "LEFT") == 0) params->left = value;
            else if (strcmp(constant, "RIGHT") == 0) params->right = value;
            else if (strcmp(constant, "UP") == 0) params->up = value;
            else if (strcmp(constant, "DOWN") == 0) params->down = value;
            else if (strcmp(constant, "CAR_MOVE") == 0) params->carMove = value;
        }
    }
}

//game constants
void readGameConstParameters(const char* filename, struct gameParameters* params) {
    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    char line[100];      // Increased to prevent overflow
    char constant[50];        // Increased for larger keys
    int value;

    while (fgets(line, sizeof(line), file)) {
        memset(constant, 0, sizeof(constant));

        if (sscanf(line, "%49[^=]=%d", constant, &value) == 2) {
            //reading game constants
            if (strcmp(constant, "STANDBY_TIME") == 0) params->standByTime = value;
            else if (strcmp(constant, "STORK_SPEED") == 0) params->storkSpeed = value;
            else if (strcmp(constant, "CARG_DISTANCE") == 0) params->distanceCarG = value;
            else if (strcmp(constant, "CART_DISTANCE") == 0) params->distanceCarT = value;
        }
    }
}

//element symbols
void readSymbolsParameters(const char* filename, struct gameParameters* params) {
    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    char line[100];
    char constant[50];
    int value;

    while (fgets(line, sizeof(line), file)) {
        memset(constant, 0, sizeof(constant));

        if (sscanf(line, "%49[^=]=%d", constant, &value) == 2) {
            //reading element symbols
            if (strcmp(constant, "FROG_SYMBOL") == 0) params->frogSymbol = value;
            else if (strcmp(constant, "CARB_SYMBOL") == 0) params->carBSymbol = value;
            else if (strcmp(constant, "CARG_SYMBOL") == 0) params->carGSymbol = value;
            else if (strcmp(constant, "CARST_SYMBOL") == 0) params->carsTSymbol = value;
            else if (strcmp(constant, "OBSTACLE_SYMBOL") == 0) params->obstacleSymbol = value;
            else if (strcmp(constant, "SIZE_BIG") == 0) params->size = value;
        }
    }
}

//array sizes
void readArrayParameters(const char* filename, struct gameParameters* params) {
    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    char line[100];
    char constant[50];
    int value;

    while (fgets(line, sizeof(line), file)) {
        memset(constant, 0, sizeof(constant));

        if (sscanf(line, "%49[^=]=%d", constant, &value) == 2) {
            //reading array sizes (game field sizes)
            if (strcmp(constant, "LANES") == 0) params->lanes = value;
            else if (strcmp(constant, "COLUMNS") == 0) params->columns = value;
            else if (strcmp(constant, "CARS") == 0) params->cars = value;
            else if (strcmp(constant, "CARS_T") == 0) params->carsT = value;
        }
    }
}

//color values
void readColorParameters(const char* filename, struct gameParameters* params) {
    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    char line[100];
    char constant[50];
    int value;

    while (fgets(line, sizeof(line), file)) {
        memset(constant, 0, sizeof(constant));

        if (sscanf(line, "%49[^=]=%d", constant, &value) == 2) {
            //reading color values
            if (strcmp(constant, "COLOR_FROG") == 0) params->colorFrog = value;
            else if (strcmp(constant, "COLOR_CARB") == 0) params->colorCarB = value;
            else if (strcmp(constant, "COLOR_CARG") == 0) params->colorCarG = value;
            else if (strcmp(constant, "COLOR_CART") == 0) params->colorCarT = value;
            else if (strcmp(constant, "COLOR_FIELD") == 0) params->colorField = value;
            else if (strcmp(constant, "COLOR_OBSTACLE") == 0) params->colorObstacle = value;
        }
    }

    fclose(file);

}

void readAllParameters(const char* filename, struct gameParameters* params) {
    readArrayParameters(filename, params);
    readBorderParameters(filename, params);
    readColorParameters(filename, params);
    readGameConstParameters(filename, params);
    readSymbolsParameters(filename, params);
    readMovesParameters(filename, params);
}

void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
}

//setting colors to elements
void setColor(int color) { //zmieniæ parametry w pliku ¿eby od razu by³o np. color_frog = 32, a nie color_frog = 2
    int ansiColor;
    switch (color) {
    case 1: ansiColor = 31; break; // czerwony
    case 2: ansiColor = 32; break; // zielony
    case 3: ansiColor = 33; break; // ¿ó³ty
    case 4: ansiColor = 34; break; // niebieski
    case 5: ansiColor = 35; break; // fioletowy
    default: ansiColor = 37; break;
    }
    printf("\033[0;%dm", ansiColor);
}

//drawing the game field
void drawBoard(char f[][COLUMNS], struct gameParameters params) {
    gotoxy(0, 0);

    for (int i = 0; i < params.lanes; i++) {
        for (int j = 0; j < params.columns; j++) {
            if (f[i][j] == params.frogSymbol) {
                setColor(params.colorFrog);
            }
            else if (f[i][j] == params.carBSymbol) {
                setColor(params.colorCarB);
            }
            else if (f[i][j] == params.carGSymbol) {
                setColor(params.colorCarG);
            }
            else if (f[i][j] == params.carsTSymbol) {
                setColor(params.colorCarT);
            }
            else if (f[i][j] == params.horizontalBorder || f[i][j] == params.verticalBorder ||
                f[i][j] == params.topLeft || f[i][j] == params.topRight ||
                f[i][j] == params.downRight || f[i][j] == params.downLeft) {
                setColor(params.colorField);
            }
            else if (f[i][j] == params.obstacleSymbol) {
                setColor(params.colorObstacle);
            }
            else {
                setColor(7);
            }
            printf("%c", f[i][j]);
        }
        printf("\n");
    }
}

//initializing fixed obstacles positions
void initializeObstacles(struct gameParameters params) {

    for (int i = 0; i < OBSTACLES; i++) {
        obstacles[i].color = params.colorObstacle;

        int row;
        do {
            row = (rand() % (params.lanes - 2)) + 1;
        } while (row % 2 == 0);

        obstacles[i].x = row;

        obstacles[i].y = (rand() % (params.columns - 2)) + 1;
    }

}

//setting a "taking" car's position
void moveCarTAtBorder(int i, struct gameParameters params) {
    carsT[i].color = params.colorCarT;
    carsT[i].lane = 2 * (1 + (rand() % ((params.lanes - 2) / 2))); // Even rows as lanes
    carsT[i].direction = (rand() % 2 == 0) ? 1 : -1; // Randomly assign left or right direction
    carsT[i].y = (carsT[i].direction == 1) ? 1 : params.columns - 2; // Start at left or right edge
    carsT[i].speed = rand() % params.carsT + 1;
    carsT[i].move = params.carMove;
    carsT[i].distanceToTake = params.distanceCarT;
    carsT[i].lastMoveTime = time(NULL);
    carsT[i].size = 1;
}

//setting a bad car's properties
void moveCarBAtBorder(int i, struct gameParameters params) {
    carsB[i].color = params.colorCarB;
    carsB[i].lane = 2 * (1 + (rand() % ((params.lanes - 2) / 2))); // Even rows as lanes
    carsB[i].direction = (rand() % 2 == 0) ? 1 : -1; // Randomly assign left or right direction
    carsB[i].y = (carsB[i].direction == 1) ? 1 : params.columns - 2; // Start at left or right edge
    carsB[i].speed = rand() % params.cars + 1;
    carsB[i].move = params.carMove;
    carsB[i].lastMoveTime = time(NULL);
    carsB[i].size = 1;
    carsB[i].speeding = rand() % 2;
}

//setting a good car's properties
void moveCarGAtBorder(int i, struct gameParameters params) {
    carsG[i].color = params.colorCarG;
    carsG[i].lane = 2 * (1 + (rand() % ((params.lanes - 2) / 2))); // Even rows as lanes
    carsG[i].direction = (rand() % 2 == 0) ? 1 : -1; // Randomly assign left or right direction
    carsG[i].y = (carsG[i].direction == 1) ? 1 : params.columns - 2; // Start at left or right edge
    carsG[i].speed = rand() % params.cars + 1;
    carsG[i].move = params.carMove;
    carsG[i].distanceToFrog = params.distanceCarG;
    carsG[i].lastMoveTime = time(NULL);
    carsB[i].size = 1;

}

//creating the border
void creatingBorder(char field[][COLUMNS], struct gameParameters params) {
    //horizontal border & special symbols for corners
    for (int r = 0; r < params.columns; r++) {
        if (r == 0) {
            field[0][0] = params.topLeft;
            field[params.lanes - 1][0] = params.downLeft;
        }
        else if (r == (params.columns - 1)) {
            field[0][params.columns - 1] = params.topRight;
            field[params.lanes - 1][params.columns - 1] = params.downRight;
        }
        else {
            field[0][r] = params.horizontalBorder;
            field[params.lanes - 1][r] = params.horizontalBorder;
        }
    }

    //vertical border
    for (int c = 1; c < params.lanes - 1; c++) {
        if (c != 0 && (c != params.lanes - 1)) {
            field[c][0] = params.verticalBorder;
            field[c][COLUMNS - 1] = params.verticalBorder;
        }
    }

    //creating lines that create lanes
    for (int i = 1; i < params.lanes - 1; i++) {
        if (i % 2 == 1) {
            for (int j = 1; j < params.columns - 1; j++) {
                field[i][j] = params.laneSymbol;
            }
        }
    }
}

//creating the default game field with frog and cars
void creatingBoard(char field[][COLUMNS], struct gameParameters params) {
    for (int i = 0; i < params.lanes; i++) {
        for (int j = 0; j < params.columns; j++) {
            field[i][j] = ' ';
        }
    }

    creatingBorder(field, params);

    //putting obstacles on the board on some lane symbols
    for (int i = 0; i < OBSTACLES; i++) {
        int obsX = obstacles[i].x;
        int obsY = obstacles[i].y;
        if (field[obsX][obsY] == params.laneSymbol) {
            field[obsX][obsY] = params.obstacleSymbol;
        }
    }

    //initializing the bad and good cars' default positions
    for (int i = 0; i < params.cars; i++) {
        moveCarBAtBorder(i, params);
        moveCarGAtBorder(i, params);
    }

    //initializing the "taking" cars' default positions
    for (int i = 0; i < params.carsT; i++) {
        moveCarTAtBorder(i, params);
    }

    //initializing frog's default position
    frog.color = params.colorFrog;
    frog.x = params.columns / 2;
    frog.y = params.lanes - 2;
}

//setting the bad cars' driving parameters
void carsBPosition(int i, struct gameParameters params) {
    if (carsB[i].direction == 1) {
        if (carsB[i].y + carsB[i].move < params.columns - 2) {
            carsB[i].y += carsB[i].move;
        }
        else {
            moveCarBAtBorder(i, params);
        }
    }
    else {
        if (carsB[i].y - carsB[i].move > 1) {
            carsB[i].y -= carsB[i].move;
        }
        else {
            moveCarBAtBorder(i, params);
        }
    }
}

//setting the good cars' driving parameters
void carsGPosition(int i, struct gameParameters params) {
    if (carsG[i].direction == 1) {
        if (carsG[i].y + carsG[i].move < params.columns - 2) {
            carsG[i].y += carsG[i].move;
        }
        else {
            moveCarGAtBorder(i, params);
        }
    }
    else {
        if (carsG[i].y - carsG[i].move > 1) {
            carsG[i].y -= carsG[i].move;
        }
        else {
            moveCarGAtBorder(i, params);
        }
    }
}

//setting the "taking" cars' driving parameters
void carsTPosition(int i, struct gameParameters params) {
    if (carsT[i].direction == 1) {
        if (carsT[i].y + carsT[i].move < params.columns - 2) {
            carsT[i].y += carsT[i].move;
        }
        else {
            moveCarTAtBorder(i, params);
        }
    }
    else {
        if (carsT[i].y - carsT[i].move > 1) {
            carsT[i].y -= carsT[i].move;
        }
        else {
            moveCarTAtBorder(i, params);
        }
    }
}

//checking distance from good car to the frog
int checkDistance(int i, struct gameParameters params) {
    int distanceY = carsG[i].y - frog.x;

    if (distanceY <= params.distanceCarG && distanceY >= -(params.distanceCarG) && carsG[i].lane == frog.y) {
        return 1;
    }
    else {
        return 0;
    }
}


//moving the car every interval (calculated for the said car)
void moveCars(struct gameParameters params) {

    double minInterval = 0.1; //fastest pace
    double maxInterval = 2.0;  //slowest pace

    for (int i = 0; i < params.cars; i++) {
        time_t currentTime = time(NULL);

        double intervalB = maxInterval - (carsB[i].speed * (maxInterval - minInterval) / params.cars);
        double intervalG = maxInterval - (carsG[i].speed * (maxInterval - minInterval) / params.cars);

        //moving bad cars each interval
        if (difftime(currentTime, carsB[i].lastMoveTime) >= intervalB) {
            carsBPosition(i, params);
            carsB[i].lastMoveTime = currentTime;
        }

        //moving good cars each interval
        if (difftime(currentTime, carsG[i].lastMoveTime) >= intervalG && (checkDistance(i, params) == 0)) {
            carsGPosition(i, params);
            carsG[i].lastMoveTime = currentTime;
        }
    }
}

//additional function because there are fewer "taking" cars than the good and bad ones
void moveCarsT(struct gameParameters params) {

    double minInterval = 0.1; //fastest pace
    double maxInterval = 2.0;  //slowest pace

    for (int i = 0; i < params.carsT; i++) {
        time_t currentTime = time(NULL);

        double intervalT = maxInterval - (carsT[i].speed * (maxInterval - minInterval) / params.carsT);

        //moving "taking" cars each interval
        if (difftime(currentTime, carsT[i].lastMoveTime) >= intervalT) {
            carsTPosition(i, params);
            carsT[i].lastMoveTime = currentTime;
        }
    }
}

int frogPosition(struct gameParameters params) {
    for (int i = 0; i < OBSTACLES; i++) {
        if (frog.y - params.up == obstacles[i].x && frog.x == obstacles[i].y) {
            return 1;
        }
        else if (frog.y + params.down == obstacles[i].x && frog.x == obstacles[i].y) {
            return 1;
        }
        else if (frog.x - params.left == obstacles[i].y && frog.y == obstacles[i].x) {
            return 1;
        }
        else if (frog.x + params.right == obstacles[i].y && frog.y == obstacles[i].x) {
            return 1;
        }
        else {
            return 0;
        }
    }
}

//controling the frog with keys
void moveFrog(struct gameParameters params) {
    if (_kbhit()) {
        //int canMove = frogPosition(params);
        char key = _getch();
        if (!frog.frogOnCar/* && canMove != 1*/) {
            if (key == 'w' && frog.y > 1) frog.y -= params.up;
            else if (key == 's' && frog.y < LANES - 2) frog.y += params.down;
            else if (key == 'a' && frog.x > 1) frog.x -= params.left;
            else if (key == 'd' && frog.x < COLUMNS - 2) frog.x += params.right;
        }


    }
}

//testing whether the frog wants to be taken by or let off the taxi
int drivingFrog(int i, struct gameParameters params, int moves) {
    moves = params.distanceCarT;
    if (_kbhit()) {
        char key = _getch();
        if (key == 't')  if (frog.frogOnCar) {
            frog.frogOnCar = 0;
            return 0;
        }
        else {
            return 1;
        }
    }
    return 0;
}

//checking if frog and any bad car are colliding
int checkCollision(struct gameParameters params) {
    for (int i = 0; i < params.cars; i++) {
        if (carsB[i].lane == frog.y) {
            if (carsB[i].y == frog.x) {
                return 1;
            }
        }

        //good car stops when the frog is near it but still you lose points if you collide with it
        if (carsG[i].lane == frog.y) {
            if (carsG[i].y == frog.x) {
                return 1;
            }
        }

        if (i < params.carsT && carsT[i].lane == frog.y && carsT[i].y == frog.x) {
            //if the frog collides with the "taking" car - it drives with it
            int taxi = drivingFrog(i, params, params.distanceCarT);
            if (taxi == 0) {
                carsT[i].color = frog.color;
                frog.frogOnCar = 1;
                return 2;
            }
            else {
                return 0;
            }
        }

    }
    return 0;
}

//updating the field with frog's and cars' moves
void update(char field[][COLUMNS], struct gameParameters params) {

    if (frog.frogOnCar) {
        for (int i = 0; i < params.carsT; i++) {
            if (carsT[i].lane == frog.y && frog.x == carsT[i].y) {
                frog.x += carsT[i].direction * carsT[i].move;

                carsT[i].distanceToTake -= carsT[i].move; // left or right - depends on the direction the car drives in

                if (carsT[i].distanceToTake <= 0) {
                    frog.frogOnCar = 0;
                    carsT[i].distanceToTake = params.distanceCarT;
                }
                break;
            }
        }
    }

    for (int i = 1; i < params.lanes - 1; i++) {
        for (int j = 1; j < params.columns - 1; j++) {
            if (field[i][j] == params.frogSymbol) {
                if (i % 2 == 1) {
                    field[i][j] = params.laneSymbol;
                }
                else {
                    field[i][j] = ' ';
                }
            }
            if (field[i][j] != params.laneSymbol && field[i][j] != params.frogSymbol
                && field[i][j] != params.obstacleSymbol) {
                field[i][j] = ' ';
            }
        }
    }

    //updating frog position
    setColor(frog.color);
    field[frog.y][frog.x] = params.frogSymbol;

    //updating good and bad cars' positions
    for (int i = 0; i < params.cars; i++) {
        setColor(carsB[i].color);
        field[carsB[i].lane][carsB[i].y] = params.carBSymbol;
        setColor(carsG[i].color);
        field[carsG[i].lane][carsG[i].y] = params.carGSymbol;
    }

    //updating "taking" cars' postions
    for (int i = 0; i < params.carsT; i++) {
        setColor(carsT[i].color);
        for (int s = 0; s < carsT[i].size; s++) {
            if (carsT[i].y + s < params.columns - 1) {
                field[carsT[i].lane][carsT[i].y + s] = params.carsTSymbol;
            }
            /*if (checkCollision(params) == 2 && params.distanceCarT > 0) {
                setColor(frog.color);
                frog.y = carsT[i].lane;
                frog.x = carsT[i].y;
                params.distanceCarT -= 1;
                if (params.distanceCarT == 0) {
                    params.distanceCarT = 5;
                }
            }*/
        }
        //field[carsT[i].lane][carsT[i].y] = params.carsTSymbol;
    }

}



//checking whether the carB is colliding with carT or carG
void checkCarCollision(struct gameParameters params) {
    for (int i = 0; i < params.cars; i++) {
        for (int j = 0; j < params.cars; j++) {
            if (carsB[i].lane == carsG[j].lane && carsB[i].y == carsG[j].y) {
                carsB[i].size = params.size;
            }
            else {
                carsB[i].size = 1;
            }
        }
        for (int j = 0; j < params.carsT; j++) {
            if (carsB[i].lane == carsT[j].lane && carsB[i].y == carsT[j].y) {
                carsB[i].size = params.size;
            }
            else {
                carsB[i].size = 1;
            }
        }
    }
}

//checking whether the carT is colliding with carT or carG
void checkCarTCollision(struct gameParameters params) {
    for (int i = 0; i < params.carsT; i++) {
        if (carsT[i].lane == frog.y && carsT[i].y == frog.x) {
            carsT[i].size = params.size;
        }
        else {
            carsT[i].size = 1;
        }
        for (int j = 0; j < params.cars; j++) {
            if (carsB[i].lane == carsT[j].lane && carsB[i].y == carsT[j].y) {
                carsT[i].size = params.size;
            }
            else {
                carsT[i].size = 1;
            }
        }
    }
}

//hiding the cursor so the bord doesn't flicker
void hideCursor() {
    printf("\033[?25l");
}

void initializeGame(char field[][COLUMNS]) {
    moveCars(parameters);
    moveCarsT(parameters);
    moveFrog(parameters);
    checkCarCollision(parameters);
    checkCarTCollision(parameters);
    update(field, parameters);
    drawBoard(field, parameters);
}

//win/loss conditions for the game
int winConditions(struct game* game, int temp, int curr, struct gameParameters params, struct Frog frog) {
    if (checkCollision(params) == 1 && game->lifes == 1) {
        game->lifes = 0;
        return 1;
    }
    else if (checkCollision(params) == 1 && game->lifes > 1 && game->collisionOccured == 0) {
        game->lifes -= 1;
        game->collisionOccured = 1;
    }
    if (temp != curr) {
        game->collisionOccured = 0;
    }
    if (temp == curr) {
        time_t currentTime = time(NULL);
        double stationaryTime = difftime(currentTime, game->lastMoveTime);

        if (stationaryTime > params.standByTime) {
            return 2;
        }
    }
    else {
        game->lastMoveTime = time(NULL);
    }

    if (temp > curr) {
        game->score += 1;
    }
    else if (temp < curr) {
        game->score -= 1;
    }

    if (frog.y == 1) {
        return 0;
    }

    return -1;
}

void Sleep(unsigned int milliseconds) {
    clock_t start_time = clock();
    while (clock() < start_time + milliseconds * CLOCKS_PER_SEC / 1000);
}

int main() {
    hideCursor();

    readAllParameters("gameParameters.txt", &parameters);

    time_t startTime = time(NULL);

    game.score = 0;
    game.lifes = 5;
    game.lastMoveTime = startTime;
    game.collisionOccured = 0;

    srand(time(NULL));

    char field[LANES][COLUMNS];
    initializeObstacles(parameters);
    creatingBoard(field, parameters);

    while (1) {
        int tempFrogPos = frog.y;
        initializeGame(field);
        int currFrogPos = frog.y;

        game.time = (int)difftime(time(NULL), startTime);

        gotoxy(0, parameters.lanes + 1);
        printf("Score: %d   Time: %d seconds   Lifes: %d\n", game.score, game.time, game.lifes);
        printf("Maria Dopiera³a, numer albumu: 203186\n");

        int condition = winConditions(&game, tempFrogPos, currFrogPos, parameters, frog);

        if (condition == 0) {
            printf("Congratulations, you won! Final score: %d\n", game.score);
            break;
        }
        else if (condition == 1) {
            printf("Collision! Game over. Final score: %d\n", game.score);
            break;
        }
        else if (condition == 2) {
            printf("You are too slow! Game over! Final score: %d\n", game.score);
            break;
        }

        Sleep(100);
    }

    printf("\033[?25h");

    return 0;
}