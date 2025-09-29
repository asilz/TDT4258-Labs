/***************************************************************************************************
 * DON'T REMOVE THE VARIABLES BELOW THIS COMMENT                                                   *
 **************************************************************************************************/
unsigned long long __attribute__((used)) VGAaddress = 0xc8000000;    // Memory storing pixels
unsigned long long __attribute__((used)) VGAaddressEnd = 0xc803ffff; // Memory storing pixels
unsigned int __attribute__((used)) red = 0x0000F0F0;
unsigned int __attribute__((used)) green = 0x00000F0F;
unsigned int __attribute__((used)) blue = 0x000000FF;
unsigned int __attribute__((used)) white = 0x0000FFFF;
unsigned int __attribute__((used)) black = 0x0;

// Don't change the name of this variables
#define NCOLS 10     // <- Supported value range: [1,18]
#define NROWS 16     // <- This variable might change.
#define TILE_SIZE 15 // <- Tile size, might change.

char *won = "You Won";                                         // DON'T TOUCH THIS - keep the string as is
char *lost = "You Lost";                                       // DON'T TOUCH THIS - keep the string as is
unsigned short height = 240;                                   // DON'T TOUCH THIS - keep the value as is
unsigned short width = 320;                                    // DON'T TOUCH THIS - keep the value as is
char font8x8[128][8];                                          // DON'T TOUCH THIS - this is a forward declaration
unsigned char tiles[NROWS][NCOLS] __attribute__((used)) = {0}; // DON'T TOUCH THIS - this is the tile map
/**************************************************************************************************/

/***
 * TODO: Define your variables below this comment
 */
#define BAR_X 7
#define BAR_Y 45

static int ball_x = 50;
static int ball_y = 120;

static int ball_angle = 0b001;

static int bar_y = 120;

/***
 * You might use and modify the struct/enum definitions below this comment
 */
typedef struct _block
{
    unsigned char destroyed;
    unsigned char deleted;
    unsigned int pos_x;
    unsigned int pos_y;
    unsigned int color;
} Block;

static Block blocks[NCOLS][NROWS];

typedef enum _gameState
{
    Stopped = 0,
    Running = 1,
    Won = 2,
    Lost = 3,
    Exit = 4,
} GameState;
GameState currentState = Stopped;

/***
 * Here follow the C declarations for our assembly functions
 */

void SetPixel(unsigned int x_coord, unsigned int y_coord, unsigned int color);
void ClearScreen();
int ReadUart();
void WriteUart(char c);

/***
 * Now follow the assembly implementations
 */

// It must only clear the VGA screen, and not clear any game state
asm("ClearScreen: \n\t"
    "    LDR R3, =#0xc8000000 \n\t" // VGA address
    "    LDR R1, =#0xc803ffff \n\t" // VGA end address
    "    MOV R0, #0 \n\t"
    "    ClearScreen_loop:"
    "    STRH R0, [R3] \n\t"
    "    ADD R3, #2 \n\t"
    "    CMP R3, R1 \n\t"
    "    BLE ClearScreen_loop \n\t"
    "    BX LR");

// assumes R0 = x-coord, R1 = y-coord, R2 = colorvalue
asm("SetPixel: \n\t"
    "LDR R3, =VGAaddress \n\t"
    "LDR R3, [R3] \n\t"
    "LSL R1, R1, #10 \n\t"
    "LSL R0, R0, #1 \n\t"
    "ADD R1, R0 \n\t"
    "STRH R2, [R3,R1] \n\t"
    "BX LR");

asm("ReadUart:\n\t"
    "LDR R1, =0xFF201000 \n\t" // JTAG UART offset
    "LDR R0, [R1]\n\t"
    "BX LR");

// TODO: Add the WriteUart assembly procedure here that respects the WriteUart C declaration on line 46
asm("WriteUart:\n\t"
    "LDR R1, =#0xFF201000 \n\t" // JTAG UART offset
    "STRB R0, [R1] \n\t"
    "BX LR");

// TODO: Implement the C functions below
// Don't modify any function header
void draw_block(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int color)
{
    for (int i = x; i < x + width; ++i)
    {
        for (int j = y; j < y + height; ++j)
        {
            if (j < 0 || i < 0 || i >= 320 || j >= 240)
            {
                continue;
            }
            SetPixel(i, j, color);
        }
    }
}

void draw_bar(unsigned int y)
{
    draw_block(0, y, BAR_X, BAR_Y, white);
}

void draw_ball(unsigned int color)
{
    SetPixel(ball_x, ball_y, color);
    for (int i = -3; i < 4; ++i)
    {
        if (ball_x + i < width && ball_x + i >= BAR_X)
        {
            SetPixel(ball_x + i, ball_y, color);
        }
        if (ball_y + i < height && ball_y + i >= 0)
        {
            SetPixel(ball_x, ball_y + i, color);
        }
    }

    if (ball_x + 1 < width && ball_x + 1 >= BAR_X && ball_y + 1 < height && ball_y + 1 >= 0)
    {
        SetPixel(ball_x + 1, ball_y + 1, color);
    }
    if (ball_x + 2 < width && ball_x + 2 >= BAR_X && ball_y + 1 < height && ball_y + 1 >= 0)
    {
        SetPixel(ball_x + 2, ball_y + 1, color);
    }
    if (ball_x + 1 < width && ball_x + 1 >= BAR_X && ball_y + 2 < height && ball_y + 2 >= 0)
    {
        SetPixel(ball_x + 1, ball_y + 2, color);
    }

    if (ball_x - 1 < width && ball_x - 1 >= BAR_X && ball_y + 1 < height && ball_y + 1 >= 0)
    {
        SetPixel(ball_x - 1, ball_y + 1, color);
    }
    if (ball_x - 2 < width && ball_x - 2 >= BAR_X && ball_y + 1 < height && ball_y + 1 >= 0)
    {
        SetPixel(ball_x - 2, ball_y + 1, color);
    }
    if (ball_x - 1 < width && ball_x - 1 >= BAR_X && ball_y + 2 < height && ball_y + 2 >= 0)
    {
        SetPixel(ball_x - 1, ball_y + 2, color);
    }

    if (ball_x + 1 < width && ball_x + 1 >= BAR_X && ball_y - 1 < height && ball_y - 1 >= 0)
    {
        SetPixel(ball_x + 1, ball_y - 1, color);
    }
    if (ball_x + 2 < width && ball_x + 2 >= BAR_X && ball_y - 1 < height && ball_y - 1 >= 0)
    {
        SetPixel(ball_x + 2, ball_y - 1, color);
    }
    if (ball_x + 1 < width && ball_x + 1 >= BAR_X && ball_y - 2 < height && ball_y - 2 >= 0)
    {
        SetPixel(ball_x + 1, ball_y - 2, color);
    }

    if (ball_x - 1 < width && ball_x - 1 >= BAR_X && ball_y - 1 < height && ball_y - 1 >= 0)
    {
        SetPixel(ball_x - 1, ball_y - 1, color);
    }
    if (ball_x - 2 < width && ball_x - 2 >= BAR_X && ball_y - 1 < height && ball_y - 1 >= 0)
    {
        SetPixel(ball_x - 2, ball_y - 1, color);
    }
    if (ball_x - 1 < width && ball_x - 1 >= BAR_X && ball_y - 2 < height && ball_y - 2 >= 0)
    {
        SetPixel(ball_x - 1, ball_y - 2, color);
    }
}

void draw_playing_field()
{
    for (int i = 0; i < NCOLS; ++i)
    {
        for (int j = 0; j < NROWS; ++j)
        {
            if (blocks[i][j].destroyed)
            {
                continue;
            }
            draw_block(blocks[i][j].pos_x, blocks[i][j].pos_y, TILE_SIZE, TILE_SIZE, blocks[i][j].color);
        }
    }
}

void update_game_state()
{
    if (currentState != Running)
    {
        return;
    }

    // TODO: Check: game won? game lost?

    // TODO: Update balls position and direction
    draw_ball(black);
    switch (ball_angle)
    {
    case 0b001: // 90
        ball_x += 1;
        break;
    case 0b100: // 45
        ball_x += 1;
        ball_y -= 1;
        break;
    case 0b000: // 0
        ball_y -= 1;
        break;
    case 0b110: // 315
        ball_y -= 1;
        ball_x -= 1;
        break;
    case 0b010: // 270
        ball_x -= 1;
        break;
    case 0b111: // 225
        ball_x -= 1;
        ball_y += 1;
        break;
    case 0b011: // 180
        ball_y += 1;
        break;
    case 0b101: // 135
        ball_y += 1;
        ball_x += 1;
        break;
    default:
        break;
    }

    if (ball_y < 0 || ball_y >= height)
    {
        ball_angle = (~ball_angle & 1) | (ball_angle & 2) | (ball_angle & 4);
        if (ball_y < 0)
        {
            ball_y = 0;
        }
        if (ball_y >= height)
        {
            ball_y = height - 1;
        }
        draw_ball(white);
        return;
    }

    draw_ball(white);

    if (ball_x < BAR_X)
    {
        currentState = Lost;
        return;
    }

    if (ball_x >= width)
    {
        currentState = Won;
        return;
    }

    if (ball_x == 7)
    {
        if (ball_y >= bar_y)
        {
            if (ball_y < bar_y + BAR_Y / 3)
            {
                ball_angle = 0b100;
            }
            else if (ball_y < bar_y + 2 * BAR_Y / 3)
            {
                ball_angle = 0b001;
            }
            else if (ball_y < bar_y + BAR_Y)
            {
                ball_angle = 0b101;
            }
        }
        return;
    }
    for (int i = 0; i < NCOLS; ++i)
    {
        for (int j = 0; j < NROWS; ++j)
        {
            if (blocks[i][j].destroyed)
            {
                continue;
            }
            if (ball_x >= blocks[i][j].pos_x && ball_x < blocks[i][j].pos_x + TILE_SIZE && ball_y >= blocks[i][j].pos_y && ball_y < blocks[i][j].pos_y + TILE_SIZE)
            {
                blocks[i][j].destroyed = 1;
                draw_block(blocks[i][j].pos_x, blocks[i][j].pos_y, TILE_SIZE, TILE_SIZE, black);
                if (!(ball_angle & 4))
                {
                    ball_angle = (~ball_angle & 1) | ((~ball_angle & 2)) | ((ball_angle & 4));
                }
                else
                {
                    if (ball_x == blocks[i][j].pos_x + TILE_SIZE - 1 || ball_x == blocks[i][j].pos_x)
                    {
                        ball_angle = (ball_angle & 1) | ((~ball_angle & 2)) | ((ball_angle & 4));
                    }
                    else
                    {
                        ball_angle = (~ball_angle & 1) | ((ball_angle & 2)) | ((ball_angle & 4));
                    }
                }
            }
        }
    }
}

void update_bar_state()
{
    while (1)
    {
        int data = ReadUart();
        if (!((data >> 15) & 1))
        {
            break;
        }
        if ((data & 0xff) == 'w' && bar_y != 0)
        {
            draw_block(0, bar_y + BAR_Y * 2 / 3, BAR_X, BAR_Y / 3, black);
            bar_y -= 15;
            draw_block(0, bar_y, BAR_X, BAR_Y / 3, white);
        }
        if ((data & 0xff) == 's' && bar_y != height - BAR_Y)
        {
            draw_block(0, bar_y, BAR_X, BAR_Y / 3, black);
            bar_y += 15;
            draw_block(0, bar_y + BAR_Y * 2 / 3, BAR_X, BAR_Y / 3, white);
        }
    }
}

void write(const char *str)
{
    while (*str)
    {
        WriteUart(*(str++));
    }
}

void play()
{
    ClearScreen();

    draw_bar(bar_y);

    for (int i = 0; i < NCOLS; ++i)
    {
        for (int j = 0; j < NROWS; ++j)
        {
            blocks[i][j].pos_x = width - TILE_SIZE * (i + 1);
            blocks[i][j].pos_y = height - TILE_SIZE * (j + 1);
            blocks[i][j].destroyed = 0;
            if (i % 2 == j % 2)
            {
                blocks[i][j].color = red;
            }
            else
            {
                blocks[i][j].color = blue;
            }
        }
    }
    // HINT: This is the main game loop
    while (1)
    {
        update_game_state();
        update_bar_state();
        if (currentState != Running)
        {
            break;
        }
        draw_playing_field();
    }
    if (currentState == Won)
    {
        write(won);
    }
    else if (currentState == Lost)
    {
        write(lost);
    }
    else if (currentState == Exit)
    {
        return;
    }
    currentState = Stopped;
}

// It must initialize the game
void reset()
{
    // Hint: This is draining the UART buffer
    int remaining = 0;
    do
    {
        unsigned long long out = ReadUart();
        if (!(out & 0x8000))
        {
            // not valid - abort reading
            break;
        }
        remaining = (out & 0xFF0000) >> 4;
    } while (remaining > 0);

    ball_x = 50;
    ball_y = 120;
    ball_angle = 0b001;
    bar_y = 120;
    currentState = Stopped;

    // TODO: You might want to reset other state in here
}

void wait_for_start()
{
    // TODO: Implement waiting behaviour until the user presses either w/s
    while (1)
    {
        int data = ReadUart();
        if (((data >> 15) & 1) && ((data & 0xff) == 'w' || (data & 0xff) == 's'))
        {
            currentState = Running;
            return;
        }
    }
}

int main(int argc, char *argv[])
{
    ClearScreen();

    if (NCOLS > 18 || NCOLS < 1)
    {
        write("Invalid number of columns");
        return 0;
    }

    // HINT: This loop allows the user to restart the game after loosing/winning the previous game
    while (1)
    {
        wait_for_start();
        play();
        reset();
        if (currentState == Exit)
        {
            break;
        }
    }
    return 0;
}

// THIS IS FOR THE OPTIONAL TASKS ONLY

// HINT: How to access the correct bitmask
// sample: to get character a's bitmask, use
// font8x8['a']
char font8x8[128][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0000 (nul)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0001
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0002
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0003
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0004
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0005
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0006
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0007
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0008
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0009
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000B
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000C
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000D
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000E
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000F
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0010
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0011
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0012
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0013
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0014
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0015
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0016
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0017
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0018
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0019
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001B
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001C
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001D
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001E
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001F
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0020 (space)
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // U+0021 (!)
    {0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0022 (")
    {0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00}, // U+0023 (#)
    {0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00}, // U+0024 ($)
    {0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00}, // U+0025 (%)
    {0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00}, // U+0026 (&)
    {0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0027 (')
    {0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00}, // U+0028 (()
    {0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00}, // U+0029 ())
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, // U+002A (*)
    {0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00}, // U+002B (+)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06}, // U+002C (,)
    {0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00}, // U+002D (-)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // U+002E (.)
    {0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00}, // U+002F (/)
    {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}, // U+0030 (0)
    {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}, // U+0031 (1)
    {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}, // U+0032 (2)
    {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}, // U+0033 (3)
    {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}, // U+0034 (4)
    {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}, // U+0035 (5)
    {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}, // U+0036 (6)
    {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}, // U+0037 (7)
    {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}, // U+0038 (8)
    {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}, // U+0039 (9)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // U+003A (:)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06}, // U+003B (;)
    {0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00}, // U+003C (<)
    {0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00}, // U+003D (=)
    {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00}, // U+003E (>)
    {0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00}, // U+003F (?)
    {0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00}, // U+0040 (@)
    {0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00}, // U+0041 (A)
    {0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00}, // U+0042 (B)
    {0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00}, // U+0043 (C)
    {0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00}, // U+0044 (D)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00}, // U+0045 (E)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00}, // U+0046 (F)
    {0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00}, // U+0047 (G)
    {0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00}, // U+0048 (H)
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+0049 (I)
    {0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00}, // U+004A (J)
    {0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00}, // U+004B (K)
    {0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00}, // U+004C (L)
    {0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00}, // U+004D (M)
    {0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00}, // U+004E (N)
    {0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00}, // U+004F (O)
    {0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00}, // U+0050 (P)
    {0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00}, // U+0051 (Q)
    {0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00}, // U+0052 (R)
    {0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00}, // U+0053 (S)
    {0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+0054 (T)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00}, // U+0055 (U)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // U+0056 (V)
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // U+0057 (W)
    {0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00}, // U+0058 (X)
    {0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00}, // U+0059 (Y)
    {0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00}, // U+005A (Z)
    {0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00}, // U+005B ([)
    {0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00}, // U+005C (\)
    {0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00}, // U+005D (])
    {0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00}, // U+005E (^)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}, // U+005F (_)
    {0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0060 (`)
    {0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00}, // U+0061 (a)
    {0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00}, // U+0062 (b)
    {0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00}, // U+0063 (c)
    {0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00}, // U+0064 (d)
    {0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00}, // U+0065 (e)
    {0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00}, // U+0066 (f)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // U+0067 (g)
    {0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00}, // U+0068 (h)
    {0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+0069 (i)
    {0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E}, // U+006A (j)
    {0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00}, // U+006B (k)
    {0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+006C (l)
    {0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00}, // U+006D (m)
    {0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00}, // U+006E (n)
    {0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00}, // U+006F (o)
    {0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F}, // U+0070 (p)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78}, // U+0071 (q)
    {0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00}, // U+0072 (r)
    {0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00}, // U+0073 (s)
    {0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00}, // U+0074 (t)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00}, // U+0075 (u)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // U+0076 (v)
    {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00}, // U+0077 (w)
    {0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00}, // U+0078 (x)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // U+0079 (y)
    {0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00}, // U+007A (z)
    {0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00}, // U+007B ({)
    {0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00}, // U+007C (|)
    {0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00}, // U+007D (})
    {0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+007E (~)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // U+007F
};
