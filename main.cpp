#include <iostream>
#include <ctime>
#include <map>
#include <graphics.h>
#include <easyx.h>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <string>
#include <wchar.h>
#include <fstream>  // 用于文件操作
#include <algorithm> 


#define ImgSize 40
#define TimerHeight 40  // 计时器区域的高度
const int MineCountHeight = 30;  // 雷数显示区域高度

using namespace std;
// 按钮结构体
struct Button
{
    int x, y, width, height;
    const wchar_t* text;
    COLORREF normalColor;
    COLORREF hoverColor;
    bool isHovered;
};

// 输入框结构体
struct Inputbox {
    int x;
    int y;
    int width;
    int height;
    const wchar_t* placeholder;
    COLORREF bgColor;
    COLORREF borderColor;
    bool isHovered;
    bool isPassword;
};

enum class OperationMode {
    Mouse,    // 鼠标操作模式
    Keyboard  // 键盘操作模式
};

// 全局变量，用于跟踪当前操作模式
OperationMode currentOperationMode = OperationMode::Mouse;

void shouye();
void zhuce();
void denglu();
void drawButton(const Button& btn);
void drawInputBox(const Inputbox& box, const wchar_t* text, bool isActive);
bool isMouseInButton(int x, int y, const Button& btn);
bool isMouseInInputBox(int x, int y, const Inputbox& box);
bool checkLogin(const wchar_t* account, const wchar_t* password);
void showGameInstructions();
void showGameControls();
void showRanking(const std::wstring& filename, const std::wstring& difficultyName);
void showRankingDifficultySelection();
void showOperationModeSelection();

// 绘制按钮（带半透明效果）
void drawButton(const Button& btn)
{
    // 创建临时图像用于按钮
    IMAGE btnImg(btn.width, btn.height);
    SetWorkingImage(&btnImg);

    // 绘制按钮背景（带透明度）
    setfillcolor(btn.isHovered ? btn.hoverColor : btn.normalColor);
    solidrectangle(0, 0, btn.width, btn.height);

    // 绘制按钮文字
    setbkmode(TRANSPARENT);
    settextcolor(WHITE);
    settextstyle(24, 0, _T("宋体"));

    int textWidth = textwidth(btn.text);
    int textHeight = textheight(btn.text);
    outtextxy((btn.width - textWidth) / 2,
        (btn.height - textHeight) / 2,
        btn.text);

    // 恢复默认绘图目标
    SetWorkingImage(NULL);

    // 绘制带透明度的按钮
    putimage(btn.x, btn.y, &btnImg, SRCPAINT);
}

// 状态模式基类和实现类
class GameState
{
public:
    virtual void handle(class GameManager& manager) = 0;
    virtual ~GameState() = default;
};

class PlayingState : public GameState
{
public:
    void handle(GameManager& manager) override {}
};

class LostState : public GameState
{
public:
    void handle(GameManager& manager) override;
};

class WonState : public GameState
{
public:
    void handle(GameManager& manager) override;
};

// 资源管理系统
class Resource
{
public:
    virtual ~Resource() = default;
    virtual bool load(const wchar_t* path) = 0;
};

// 视图
class ImageResource : public Resource
{
private:
    IMAGE img;
public:
    bool load(const wchar_t* path) override
    {
        // 调用无返回值的 loadimage
        loadimage(&img, path, ImgSize, ImgSize);

        // 通过检查图像尺寸判断是否加载成功
        return (img.getwidth() == ImgSize && img.getheight() == ImgSize);
    }
    IMAGE* getImage() { return &img; }
};

class ResourceManager
{
private:
    static ResourceManager* instance;
    map<wstring, Resource*> resources;

    ResourceManager() { loadResources(); }

    void loadResources()
    {
        for (int i = 0; i < 12; i++) {
            wchar_t key[20], path[50];
            swprintf_s(key, L"img_%d", i);
            swprintf_s(
                path, L"./images/%d.jpg", i);

            ImageResource* imgRes = new ImageResource();
            if (imgRes->load(path)) {
                resources[key] = imgRes;
            }
        }
    }

public:
    static ResourceManager* getInstance()
    {
        if (!instance) instance = new ResourceManager();
        return instance;
    }

    ImageResource* getImage(const wchar_t* key)
    {
        auto it = resources.find(key);
        return (it != resources.end()) ? static_cast<ImageResource*>(it->second) : nullptr;
    }

    template<typename T>
    T* getResource(const wchar_t* key)
    {
        auto it = resources.find(key);
        return (it != resources.end()) ? static_cast<T*>(it->second) : nullptr;
    }


    ~ResourceManager() // 增加的析构
    {
        for (auto& pair : resources) {
            delete pair.second;
        }
    }
};
ResourceManager* ResourceManager::instance = nullptr;

// 游戏对象继承体系
class Game
{
public:
    virtual ~Game() = default;
    virtual void init() = 0;
    virtual void draw() = 0;
    virtual void handleEvent(const ExMessage& msg) = 0;
    virtual GameState* checkGameState(int r = -1, int c = -1) = 0;
    virtual int getFlagCount() const = 0;
    virtual int getRows() const = 0;
    virtual int getCols() const = 0;
};

class MinesweeperGame : public Game
{
private:
    int Rows;  // 行数
    int Cols;  // 列数
    int mineNum; // 地雷数
    vector<vector<int>> map; // 使用动态二维数组
    ResourceManager* resMgr;

    void generateMines()
    {
        for (int i = 0; i < mineNum;) {
            int r = rand() % Rows;
            int c = rand() % Cols;
            if (map[r][c] == 0) {
                map[r][c] = -1;
                i++;
            }
        }
    }

    void calculateNumbers()
    {
        for (int i = 0; i < Rows; i++) {
            for (int j = 0; j < Cols; j++) {
                if (map[i][j] == -1) {
                    for (int r = i - 1; r <= i + 1; r++) {
                        for (int c = j - 1; c <= j + 1; c++) {
                            if ((r >= 0 && r < Rows && c >= 0 && c < Cols) && map[r][c] != -1) {
                                ++map[r][c];
                            }
                        }
                    }
                }
            }
        }
    }

    void revealMines()
    {
        for (int i = 0; i < Rows; i++) {
            for (int j = 0; j < Cols; j++) {
                if (map[i][j] == 19) map[i][j] = -1;
            }
        }
    }

public:
    void boomBlank(int row, int col)
    {
        if (map[row][col] == 0) {
            for (int r = row - 1; r <= row + 1; r++) {
                for (int c = col - 1; c <= col + 1; c++) {
                    if (r >= 0 && r < Rows && c >= 0 && c < Cols) {
                        if (map[r][c] >= 19 && map[r][c] <= 28) {
                            map[r][c] -= 20;
                            boomBlank(r, c);
                        }
                    }
                }
            }
        }
    }

    int getMapValue(int row, int col) const {
        if (row >= 0 && row < Rows && col >= 0 && col < Cols) {
            return map[row][col];
        }
        return -1; // 无效位置返回-1
    }

    void setMapValue(int row, int col, int value) {
        if (row >= 0 && row < Rows && col >= 0 && col < Cols) {
            map[row][col] = value;
        }
    }

    MinesweeperGame(int r, int c, int m)
        : Rows(r), Cols(c), mineNum(m),
        map(r, vector<int>(c, 0)),  // 初始化动态数组
        resMgr(ResourceManager::getInstance())
    {
        srand(static_cast<unsigned int>(time(NULL)));
        init();
    }

    void init() override
    {
        // 初始化map为全0
        for (int i = 0; i < Rows; i++) {
            for (int j = 0; j < Cols; j++) {
                map[i][j] = 0;
            }
        }
        generateMines();
        calculateNumbers();

        // 加密地图
        for (int i = 0; i < Rows; i++) {
            for (int j = 0; j < Cols; j++) {
                map[i][j] += 20;
            }
        }
    }

    void draw() override
    {
        for (int i = 0; i < Rows; i++) {
            for (int j = 0; j < Cols; j++) {
                int value = map[i][j];
                ImageResource* imgRes = nullptr;

                if (value >= 0 && value <= 8) {
                    wchar_t key[20];
                    swprintf_s(key, L"img_%d", value);
                    imgRes = resMgr->getImage(key);
                }
                else if (value == -1) {
                    imgRes = resMgr->getImage(L"img_9");
                }
                else if (value >= 19 && value <= 28) {
                    imgRes = resMgr->getImage(L"img_10");
                }
                else if (value >= 39) {
                    imgRes = resMgr->getImage(L"img_11");
                }
                if (imgRes) putimage(j * ImgSize, i * ImgSize, imgRes->getImage());
            }
        }
    }

    void handleEvent(const ExMessage& msg) override
    {
        int r = msg.y / ImgSize;
        int c = msg.x / ImgSize;

        // 忽略计时器区域的点击
        if (r < 0 || r >= Rows || c < 0 || c >= Cols) return;

        if (msg.message == WM_LBUTTONDOWN) {
            if (map[r][c] >= 19 && map[r][c] <= 28) {
                map[r][c] -= 20;
                boomBlank(r, c);
            }
        }
        else if (msg.message == WM_RBUTTONDOWN) {
            // 计算当前已标记的旗帜数量
            int currentFlagCount = 0;
            for (int i = 0; i < Rows; i++) {
                for (int j = 0; j < Cols; j++) {
                    if (map[i][j] >= 39) {
                        currentFlagCount++;
                    }
                }
            }

            if (map[r][c] >= 19 && map[r][c] <= 28) {
                // 只有当旗帜数量小于地雷总数时才允许标记
                if (currentFlagCount < mineNum) {
                    map[r][c] += 20;
                }
            }
            else if (map[r][c] >= 39) {
                // 取消标记总是允许的
                map[r][c] -= 20;
            }
        }
    }

    GameState* checkGameState(int clickR = -1, int clickC = -1) override
    {
        if (clickR != -1 && clickC != -1) {
            // 检查点击位置是否有效
            if (clickR < 0 || clickR >= Rows || clickC < 0 || clickC >= Cols)
                return new PlayingState();

            int value = map[clickR][clickC];
            if (value == -1 || value == 19)
            {
                revealMines();
                return new LostState();
            }
        }

        int openedCount = 0;
        for (int i = 0; i < Rows; i++)
        {
            for (int j = 0; j < Cols; j++) {
                if (map[i][j] >= 0 && map[i][j] <= 8) openedCount++;
            }
        }
        if (openedCount == Rows * Cols - mineNum) {
            return new WonState();
        }
        return new PlayingState();
    }

    int getRows() const { return Rows; }
    int getCols() const { return Cols; }
    int getFlagCount() const {
        int flagCount = 0;
        for (int i = 0; i < Rows; i++) {
            for (int j = 0; j < Cols; j++) {
                if (map[i][j] >= 39) {  // 39及以上表示被标记的格子
                    flagCount++;
                }
            }
        }
        return flagCount;
    }
};

// 游戏管理系统
class GameManager
{
private:
    int Rows, Cols, mineNum;
    Game* currentGame;
    GameState* currentState;
    clock_t startTime;      // 游戏开始时间
    bool isTiming;          // 是否正在计时
    double bestTime;        // 保存最佳时间（胜利时的时间）
    bool hasBestTime;       // 是否有保存的最佳时间
    std::wstring currentAccount; // 当前登录的账号
    int remainingMines;  // 剩余雷数
    OperationMode currentOperationMode;

    // 获取对应难度的文件名
    std::wstring getDifficultyFileName() const {
        if (Rows == 10 && Cols == 10) {
            return L"easy_records.txt";
        }
        else if (Rows == 15 && Cols == 15) {
            return L"medium_records.txt";
        }
        else if (Rows == 20 && Cols == 20) {
            return L"hard_records.txt";
        }
        return L"";
    }

    // 从文件加载最佳时间
    void loadBestTime() {
        std::wifstream inFile(getDifficultyFileName());

        if (inFile) {
            std::wstring line;
            while (std::getline(inFile, line)) {
                // 格式: 账号:难度:时间
                size_t pos1 = line.find(L':');
                size_t pos2 = line.find(L':', pos1 + 1);
                if (pos1 != std::wstring::npos && pos2 != std::wstring::npos) {
                    std::wstring account = line.substr(0, pos1);
                    std::wstring difficulty = line.substr(pos1 + 1, pos2 - pos1 - 1);
                    // 检查是否是当前账号和当前难度
                    if (account == currentAccount &&
                        difficulty == std::to_wstring(Rows) + L"x" + std::to_wstring(Cols)) {
                        try {
                            bestTime = std::stod(line.substr(pos2 + 1));
                            hasBestTime = true;
                            break;
                        }
                        catch (...) {
                            // 转换失败时保持默认值
                            bestTime = 0.0;
                            hasBestTime = false;
                        }
                    }
                }
            }
            inFile.close();
        }
        else {
            hasBestTime = false;
            bestTime = 0.0;
        }
    }
    // 保存最佳时间到文件
    void saveBestTimeToFile() {
        std::wifstream inFile(getDifficultyFileName());
        std::vector<std::wstring> lines;
        bool accountFound = false;
        // 读取现有记录
        if (inFile) {
            std::wstring line;
            while (std::getline(inFile, line)) {
                size_t pos1 = line.find(L':');
                size_t pos2 = line.find(L':', pos1 + 1);
                if (pos1 != std::wstring::npos && pos2 != std::wstring::npos) {
                    std::wstring account = line.substr(0, pos1);
                    std::wstring difficulty = line.substr(pos1 + 1, pos2 - pos1 - 1);
                    // 如果是当前账号和当前难度，更新记录
                    if (account == currentAccount &&
                        difficulty == std::to_wstring(Rows) + L"x" + std::to_wstring(Cols)) {

                        line = account + L":" + difficulty + L":" + std::to_wstring(bestTime);
                        accountFound = true;
                    }
                }
                lines.push_back(line);
            }
            inFile.close();
        }
        // 如果账号未找到，添加新记录
        if (!accountFound) {
            lines.push_back(currentAccount + L":" + std::to_wstring(Rows) + L"x" +
                std::to_wstring(Cols) + L":" + std::to_wstring(bestTime));
        }
        // 写回文件
        std::wofstream outFile(getDifficultyFileName());
        if (outFile) {
            for (const auto& line : lines) {
                outFile << line << std::endl;
            }
            outFile.close();
        }
    }
    // 绘制雷数显示
    void drawMineCount() {
        wchar_t mineStr[50];
        swprintf_s(mineStr, L"Mines: %d", remainingMines);

        // 设置雷数显示区域背景
        setfillcolor(DARKGRAY);
        solidrectangle(0, Rows * ImgSize + TimerHeight,
            Cols * ImgSize, Rows * ImgSize + TimerHeight + MineCountHeight);

        // 设置文字样式
        setbkmode(TRANSPARENT);
        settextcolor(WHITE);
        settextstyle(24, 0, L"Consolas");

        // 居中显示雷数
        int textWidth = textwidth(mineStr);
        int x = (Cols * ImgSize - textWidth) / 2;
        int y = Rows * ImgSize + TimerHeight + (MineCountHeight - 24) / 2;
        outtextxy(x, y, mineStr);
    }

public:
    GameManager(int r, int c, int m, const std::wstring& account = L"")
        : Rows(r), Cols(c), mineNum(m),
        currentGame(new MinesweeperGame(r, c, m)),
        currentState(new PlayingState()),
        startTime(0.0), isTiming(false),
        bestTime(0.0), hasBestTime(false),
        currentAccount(account), remainingMines(m),
        currentOperationMode(OperationMode::Mouse)
    {
        loadBestTime();
    }

    ~GameManager() {
        delete currentGame;
        delete currentState;
    }

    void startTimer() {
        startTime = clock();
        isTiming = true;
    }

    void stopTimer() {
        isTiming = false;
    }

    double getElapsedTime() const {
        if (startTime == 0) return 0.0; // 未开始计时
        return static_cast<double>(clock() - startTime) / CLOCKS_PER_SEC;
    }

    // 更新最佳时间（仅在胜利时调用）
    void updateBestTime() {
        double currentTime = getElapsedTime();

        if (!hasBestTime || currentTime < bestTime) {
            bestTime = currentTime;
            hasBestTime = true;
            saveBestTimeToFile(); // 保存到文件
        }
    }
    double getBestTime() const {
        return bestTime;
    }
    void resetTimer() {
        startTime = clock();
    }

    bool isTimerRunning() const { return isTiming; }

    void drawTimer() {
        double elapsed = getElapsedTime();
        wchar_t timeStr[50];
        if (!hasBestTime) {
            swprintf_s(timeStr, L"Time: %.1f s | Best: 0.0 s", elapsed);
        }
        else {
            swprintf_s(timeStr, L"Time: %.1f s | Best: %.1f s", elapsed, bestTime);
        }

        // 设置计时器区域背景
        setfillcolor(DARKGRAY);
        solidrectangle(0, Rows * ImgSize, Cols * ImgSize, Rows * ImgSize + TimerHeight);

        // 设置文字样式
        setbkmode(TRANSPARENT);
        settextcolor(WHITE);
        settextstyle(24, 0, L"Consolas");

        // 居中显示计时器
        int textWidth = textwidth(timeStr);
        int x = (Cols * ImgSize - textWidth) / 2;
        int y = Rows * ImgSize + (TimerHeight - 24) / 2;
        outtextxy(x, y, timeStr);
    }

    void run() {
        int width = Cols * ImgSize;
        int height = Rows * ImgSize + TimerHeight + MineCountHeight;

        initgraph(width, height, EW_SHOWCONSOLE);
        startTimer(); // 游戏开始时启动计时器

        int cursorX = 0, cursorY = 0; // 当前选中的格子坐标
        static clock_t lastKeyTime = 0;

        while (true) {
            // 首先处理键盘事件
            if (currentOperationMode == OperationMode::Keyboard) {
                clock_t currentTime = clock();

                // 防抖处理，100ms间隔
                if (currentTime - lastKeyTime > 100) {
                    lastKeyTime = currentTime;

                    // 方向键移动光标
                    if (GetKeyState(VK_UP) & 0x8000 && cursorY > 0) cursorY--;
                    if (GetKeyState(VK_DOWN) & 0x8000 && cursorY < Rows - 1) cursorY++;
                    if (GetKeyState(VK_LEFT) & 0x8000 && cursorX > 0) cursorX--;
                    if (GetKeyState(VK_RIGHT) & 0x8000 && cursorX < Cols - 1) cursorX++;

                    // Enter键模拟左键点击
                    if (GetKeyState(VK_RETURN) & 0x8000) {
                        int prevFlags = currentGame->getFlagCount();
                        handleLeftClick(cursorY, cursorX);
                        int currentFlags = currentGame->getFlagCount();

                        // 更新剩余雷数
                        if (currentFlags != prevFlags) {
                            remainingMines = mineNum - currentFlags;
                            if (remainingMines < 0) remainingMines = 0;
                        }

                        // 检查游戏状态
                        GameState* newState = currentGame->checkGameState(cursorY, cursorX);
                        if (dynamic_cast<LostState*>(newState)) {
                            stopTimer();
                            delete currentState;
                            currentState = newState;
                            currentState->handle(*this);
                            continue;
                        }
                        else if (dynamic_cast<WonState*>(newState)) {
                            updateBestTime();
                            stopTimer();
                            delete currentState;
                            currentState = newState;
                            currentState->handle(*this);
                            continue;
                        }
                        else {
                            delete newState;
                        }
                    }

                    // 空格键模拟右键点击
                    if (GetKeyState(VK_SPACE) & 0x8000) {
                        int prevFlags = currentGame->getFlagCount();
                        handleRightClick(cursorY, cursorX);
                        int currentFlags = currentGame->getFlagCount();

                        // 更新剩余雷数
                        if (currentFlags != prevFlags) {
                            remainingMines = mineNum - currentFlags;
                            if (remainingMines < 0) remainingMines = 0;
                        }
                    }
                }
            }


            ExMessage msg;
            while (peekmessage(&msg, EM_MOUSE)) { // 检测鼠标事件
                if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN) {
                    int prevFlags = currentGame->getFlagCount();
                    currentGame->handleEvent(msg);
                    int currentFlags = currentGame->getFlagCount();
                    if (currentFlags != prevFlags) {
                        remainingMines = mineNum - currentFlags;
                        if (remainingMines < 0) remainingMines = 0;
                    }

                    if (msg.message == WM_LBUTTONDOWN) {
                        int r = msg.y / ImgSize;
                        int c = msg.x / ImgSize;

                        // 忽略计时器区域的点击
                        if (r < Rows && c < Cols) {
                            GameState* newState = currentGame->checkGameState(r, c);

                            // 检查游戏是否结束
                            if (dynamic_cast<LostState*>(newState)) {
                                stopTimer(); // 游戏结束时停止计时器
                                delete currentState;
                                currentState = newState;
                                currentState->handle(*this);
                                continue;
                            }
                            else if (dynamic_cast<WonState*>(newState)) {
                                updateBestTime(); // 更新最佳时间
                                stopTimer(); // 游戏结束时停止计时器
                                delete currentState;
                                currentState = newState;
                                currentState->handle(*this);
                                continue;
                            }

                            delete currentState;
                            currentState = newState;
                            currentState->handle(*this);
                        }
                    }
                }
            }

            
            // 绘制游戏和计时器
            currentGame->draw();
            // 绘制键盘模式下的光标
            if (currentOperationMode == OperationMode::Keyboard) {
                setlinecolor(RED);
                rectangle(cursorX * ImgSize, cursorY * ImgSize,
                    (cursorX + 1) * ImgSize, (cursorY + 1) * ImgSize);
            }
            drawTimer();
            drawMineCount();
            FlushBatchDraw(); // 刷新显示
        }
    }

    // 重新开始游戏
    void restartGame() {
        delete currentGame;
        currentGame = new MinesweeperGame(Rows, Cols, mineNum);
        delete currentState;
        currentState = new PlayingState();
        remainingMines = mineNum; // 重置剩余雷数
        resetTimer(); // 重置计时器
        startTimer(); // 开始计时
    }

    void handleKeyboardInput(const ExMessage& msg)
    {
        if (currentOperationMode == OperationMode::Keyboard) {
            int r = msg.y / ImgSize;
            int c = msg.x / ImgSize;

            // 忽略计时器区域的点击
            if (r < Rows && c < Cols) {
                if (msg.message == WM_LBUTTONDOWN) {
                    // 自定义键盘按下时执行左键操作
                    handleLeftClick(r, c);
                }
                else if (msg.message == WM_RBUTTONDOWN) {
                    // 自定义键盘按下时执行右键操作
                    handleRightClick(r, c);
                }
            }
        }
    }

    void handleLeftClick(int row, int col) {
        MinesweeperGame* minesweeperGame = dynamic_cast<MinesweeperGame*>(currentGame);
        if (minesweeperGame) {
            int currentValue = minesweeperGame->getMapValue(row, col);
            if (currentValue >= 19 && currentValue <= 28) {
                minesweeperGame->setMapValue(row, col, currentValue - 20);
                minesweeperGame->boomBlank(row, col);
            }
            else if (currentValue >= 39) {
                // 如果点击的是标记的格子，取消标记
                if (currentGame->getFlagCount() > 0) {
                    minesweeperGame->setMapValue(row, col, currentValue - 20);
                }
            }
        }
    }

    void handleRightClick(int row, int col)
    {
        MinesweeperGame* minesweeperGame = dynamic_cast<MinesweeperGame*>(currentGame);
        if (minesweeperGame) {
            int currentValue = minesweeperGame->getMapValue(row, col);
            int currentFlagCount = currentGame->getFlagCount();

            if (currentValue >= 19 && currentValue <= 28) {
                if (currentFlagCount < mineNum) {
                    minesweeperGame->setMapValue(row, col, currentValue + 20);
                }
            }
            else if (currentValue >= 39) {
                minesweeperGame->setMapValue(row, col, currentValue - 20);
            }
        }
    }

    Game& getGame() { return *currentGame; }
};

// 显示难度选择对话框
void showRankingDifficultySelection() {
    initgraph(600, 600);
    BeginBatchDraw();

    // 定义按钮
    Button easyBtn = { 150, 180, 300, 50, L"初级 (10x10)", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button mediumBtn = { 150, 250, 300, 50, L"中级 (15x15)", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button hardBtn = { 150, 320, 300, 50, L"高级 (20x20)", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button backBtn = { 150, 400, 300, 30, L"返回", RGB(200, 200, 200), RGB(220, 220, 220), false };

    while (true) {
        cleardevice();
        setfillcolor(RGB(255, 193, 193));
        solidrectangle(0, 0, 600, 600);

        // 绘制标题
        settextcolor(WHITE);
        settextstyle(48, 0, _T("黑体"));
        outtextxy(150, 50, _T("选择难度"));

        // 绘制按钮
        drawButton(easyBtn);
        drawButton(mediumBtn);
        drawButton(hardBtn);
        drawButton(backBtn);

        FlushBatchDraw();

        MOUSEMSG msg;
        if (MouseHit()) {
            msg = GetMouseMsg();

            // 检查鼠标悬停状态
            easyBtn.isHovered = isMouseInButton(msg.x, msg.y, easyBtn);
            mediumBtn.isHovered = isMouseInButton(msg.x, msg.y, mediumBtn);
            hardBtn.isHovered = isMouseInButton(msg.x, msg.y, hardBtn);
            backBtn.isHovered = isMouseInButton(msg.x, msg.y, backBtn);

            if (msg.uMsg == WM_LBUTTONDOWN) {
                if (easyBtn.isHovered) {
                    EndBatchDraw();
                    closegraph();
                    showRanking(L"easy_records.txt", L"初级 (10x10)");
                    return;
                }
                else if (mediumBtn.isHovered) {
                    EndBatchDraw();
                    closegraph();
                    showRanking(L"medium_records.txt", L"中级 (15x15)");
                    return;
                }
                else if (hardBtn.isHovered) {
                    EndBatchDraw();
                    closegraph();
                    showRanking(L"hard_records.txt", L"高级 (20x20)");
                    return;
                }
                else if (backBtn.isHovered) {
                    EndBatchDraw();
                    closegraph();

                    shouye();
                }
            }
        }
        Sleep(10);
    }
    EndBatchDraw();
    closegraph();
}

// 自定义难度
void diygame(int& rows, int& cols, int& mineNum) {
    initgraph(600, 600);
    BeginBatchDraw();

    // 背景色
    setfillcolor(RGB(255, 193, 193));
    solidrectangle(0, 0, 600, 600);

    // 标题
    settextcolor(WHITE);
    settextstyle(48, 0, _T("黑体"));
    outtextxy(150, 50, _T("自定义游戏"));
    settextstyle(24, 0, _T("宋体"));
    outtextxy(150, 110, _T("格子数 (5-20)  炸弹数 (1-80)"));

    // 定义输入框
    Inputbox sizeBox = { 150, 180, 300, 40, L"输入格子数",
                      RGB(255, 255, 255), RGB(173, 216, 230), false, false };
    Inputbox mineBox = { 150, 250, 300, 40, L"输入炸弹数",
                       RGB(255, 255, 255), RGB(173, 216, 230), false, false };

    // 定义按钮
    Button confirmBtn = { 150, 320, 300, 50, L"确认", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button returnBtn = { 150, 390, 300, 50, L"返回", RGB(173, 216, 230), RGB(175, 238, 238), false };

    // 输入文本缓冲区
    wchar_t sizeText[64] = { 0 };
    wchar_t mineText[64] = { 0 };
    bool isInputSize = false;
    bool isInputMine = false;

    // 主循环
    while (true) {
        // 绘制界面
        cleardevice();
        setfillcolor(RGB(255, 193, 193));
        solidrectangle(0, 0, 600, 600);

        // 绘制标题
        settextcolor(WHITE);
        settextstyle(48, 0, _T("黑体"));
        outtextxy(150, 50, _T("自定义游戏"));
        settextstyle(24, 0, _T("宋体"));
        outtextxy(150, 110, _T("格子数 (5-20)  炸弹数 (1-80)"));

        // 绘制输入框
        drawInputBox(sizeBox, sizeText, isInputSize);
        drawInputBox(mineBox, mineText, isInputMine);

        // 绘制按钮
        drawButton(confirmBtn);
        drawButton(returnBtn);

        // 刷新显示
        FlushBatchDraw();

        // 处理鼠标消息
        MOUSEMSG msg;
        while (MouseHit()) {
            msg = GetMouseMsg();

            // 更新悬停状态
            sizeBox.isHovered = isMouseInInputBox(msg.x, msg.y, sizeBox);
            mineBox.isHovered = isMouseInInputBox(msg.x, msg.y, mineBox);
            confirmBtn.isHovered = isMouseInButton(msg.x, msg.y, confirmBtn);
            returnBtn.isHovered = isMouseInButton(msg.x, msg.y, returnBtn);

            // 处理点击
            if (msg.uMsg == WM_LBUTTONDOWN) {
                if (sizeBox.isHovered) {
                    isInputSize = true;
                    isInputMine = false;
                }
                else if (mineBox.isHovered) {
                    isInputSize = false;
                    isInputMine = true;
                }
                else if (confirmBtn.isHovered) {
                    // 检查输入是否有效
                    if (wcslen(sizeText) == 0 || wcslen(mineText) == 0) {
                        MessageBox(NULL, _T("请输入完整信息"), _T("提示"), MB_OK);
                        continue;
                    }

                    // 转换为数字
                    rows = cols = _wtoi(sizeText);
                    mineNum = _wtoi(mineText);

                    // 验证范围
                    if (rows < 5 || rows > 20) {
                        MessageBox(NULL, _T("格子数必须在5-20之间"), _T("提示"), MB_OK);
                        continue;
                    }
                    if (mineNum < 1 || mineNum > 80) {
                        MessageBox(NULL, _T("炸弹数必须在1-80之间"), _T("提示"), MB_OK);
                        continue;
                    }
                    if (mineNum >= rows * cols) {
                        MessageBox(NULL, _T("炸弹数不能超过格子总数"), _T("提示"), MB_OK);
                        continue;
                    }
                    // 关闭窗口并返回
                    EndBatchDraw();
                    closegraph();

                    GameManager gameManager(rows, cols, mineNum);
                    gameManager.run();
                    delete ResourceManager::getInstance(); // 新增
                    return;
                }
                else if (returnBtn.isHovered) {
                    // 返回时不修改参数
                    EndBatchDraw();
                    closegraph();
                    return;
                }
                else {
                    // 点击其他地方，取消输入状态
                    isInputSize = false;
                    isInputMine = false;
                }
            }
        }

        // 处理键盘输入（与示例代码相同的逻辑）
        if (isInputSize || isInputMine) {
            // 检查退格键
            if (GetAsyncKeyState(VK_BACK) & 0x8000) {
                if (isInputSize && wcslen(sizeText) > 0) {
                    sizeText[wcslen(sizeText) - 1] = L'\0';
                    Sleep(100); // 防止连续删除
                }
                else if (isInputMine && wcslen(mineText) > 0) {
                    mineText[wcslen(mineText) - 1] = L'\0';
                    Sleep(100);
                }
            }
            // 检查Tab键切换
            else if (GetAsyncKeyState(VK_TAB) & 0x8000) {
                isInputSize = !isInputSize;
                isInputMine = !isInputMine;
                Sleep(100);
            }
            // 检查普通字符输入
            else {
                for (int ch = 0x20; ch <= 0x7E; ch++) { // ASCII可打印字符
                    if (GetAsyncKeyState(ch) & 0x8000) {
                        if (isInputSize && wcslen(sizeText) < 63) {
                            size_t len = wcslen(sizeText);
                            sizeText[len] = (wchar_t)ch;
                            sizeText[len + 1] = L'\0';
                            Sleep(100); // 防止重复输入
                        }
                        else if (isInputMine && wcslen(mineText) < 63) {
                            size_t len = wcslen(mineText);
                            mineText[len] = (wchar_t)ch;
                            mineText[len + 1] = L'\0';
                            Sleep(100);
                        }
                    }
                }
            }
        }

        // 延时
        Sleep(10);
    }
}

void showDifficultyDialog(int& rows, int& cols, int& mineNum) {
    // 初始化窗口
    initgraph(600, 600);

    // 启用双缓冲
    BeginBatchDraw();

    // 定义按钮
    Button easyBtn = { 150, 180, 300, 50, L"初级 (10×10)", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button mediumBtn = { 150, 250, 300, 50, L"中级 (15×15)", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button hardBtn = { 150, 320, 300, 50, L"高级 (20×20)", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button diyBtn = { 150, 390, 300, 50, L"自定义游戏", RGB(173, 216, 230), RGB(175, 238, 238), false };

    while (true) {
        // 清屏并绘制背景
        cleardevice();
        setfillcolor(RGB(255, 193, 193)); // 背景颜色
        solidrectangle(0, 0, 600, 600);

        // 标题
        settextcolor(WHITE);
        settextstyle(48, 0, _T("黑体"));
        outtextxy(150, 50, _T("扫雷游戏"));

        // 提示文字
        settextstyle(24, 0, _T("宋体"));
        outtextxy(150, 130, _T("请选择难度:"));

        // 绘制按钮
        drawButton(easyBtn);
        drawButton(mediumBtn);
        drawButton(hardBtn);
        drawButton(diyBtn);

        // 刷新画面
        FlushBatchDraw();

        // 获取鼠标消息
        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();

            // 更新悬停状态
            easyBtn.isHovered = isMouseInButton(msg.x, msg.y, easyBtn);
            mediumBtn.isHovered = isMouseInButton(msg.x, msg.y, mediumBtn);
            hardBtn.isHovered = isMouseInButton(msg.x, msg.y, hardBtn);
            diyBtn.isHovered = isMouseInButton(msg.x, msg.y, diyBtn);

            // 处理点击事件
            if (msg.uMsg == WM_LBUTTONDOWN) {
                if (easyBtn.isHovered) {
                    rows = cols = 10;
                    mineNum = 15; // 初级：10x10，15个雷
                    break;
                }
                else if (mediumBtn.isHovered) {
                    rows = cols = 15;
                    mineNum = 40; // 中级：15x15，40个雷
                    break;
                }
                else if (hardBtn.isHovered)
                {
                    rows = cols = 20;
                    mineNum = 80; // 高级：20x20，80个雷
                    break;
                }
                else if (diyBtn.isHovered)
                {
                    diygame(rows, cols, mineNum);
                    break;
                }
            }
        }

        Sleep(10); // 防止CPU占用过高
    }

    EndBatchDraw(); // 关闭双缓冲
    closegraph();   // 关闭图形窗口
}

// 显示排行榜
void showRanking(const std::wstring& filename, const std::wstring& difficultyName) {
    // 读取文件内容并排序
    std::vector<std::pair<std::wstring, double>> records;

    std::wifstream inFile(filename);
    if (inFile) {
        std::wstring line;
        while (std::getline(inFile, line)) {
            size_t pos1 = line.find(L':');
            size_t pos2 = line.find(L':', pos1 + 1);
            if (pos1 != std::wstring::npos && pos2 != std::wstring::npos) {
                std::wstring account = line.substr(0, pos1);
                try {
                    double time = std::stod(line.substr(pos2 + 1));
                    records.emplace_back(account, time);
                }
                catch (...) {
                    // 忽略格式错误的行
                }
            }
        }
        inFile.close();
    }

    // 按时间从小到大排序
    std::sort(records.begin(), records.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    // 显示排行榜窗口
    initgraph(600, 600);
    BeginBatchDraw();

    // 返回按钮
    Button backBtn = { 250, 450, 100, 30, L"返回", RGB(200, 200, 200), RGB(220, 220, 220), false };

    while (true) {
        cleardevice();
        setfillcolor(RGB(255, 193, 193));
        solidrectangle(0, 0, 600, 600);

        // 绘制标题
        settextcolor(WHITE);
        settextstyle(48, 0, _T("黑体"));
        outtextxy(100, 50, difficultyName.c_str());
        outtextxy(150, 100, L"排行榜");

        // 绘制表头
        settextstyle(24, 0, _T("宋体"));
        outtextxy(100, 150, L"排名");
        outtextxy(200, 150, L"账号");
        outtextxy(400, 150, L"时间(秒)");

        // 绘制记录
        settextstyle(20, 0, _T("宋体"));
        for (size_t i = 0; i < records.size() && i < 10; i++) {
            // 排名
            std::wstring rank = std::to_wstring(i + 1) + L".";
            outtextxy(100, 200 + i * 30, rank.c_str());

            // 账号
            outtextxy(200, 200 + i * 30, records[i].first.c_str());

            // 时间
            std::wstring timeStr = std::to_wstring(records[i].second);
            // 保留一位小数
            timeStr = timeStr.substr(0, timeStr.find(L'.') + 2);
            outtextxy(400, 200 + i * 30, timeStr.c_str());
        }

        // 如果没有记录
        if (records.empty()) {
            outtextxy(200, 250, L"暂无记录");
        }
        // 绘制返回按钮
        drawButton(backBtn);

        FlushBatchDraw();

        MOUSEMSG msg;
        if (MouseHit()) {
            msg = GetMouseMsg();
            backBtn.isHovered = isMouseInButton(msg.x, msg.y, backBtn);

            if (msg.uMsg == WM_LBUTTONDOWN && backBtn.isHovered) {

                break;
            }
        }
        Sleep(10);
    }

    EndBatchDraw();
    closegraph();
    showRankingDifficultySelection(); // 返回难度选择界面
}

// 检查鼠标是否在按钮上
bool isMouseInButton(int x, int y, const Button& btn) {
    return x >= btn.x && x <= btn.x + btn.width &&
        y >= btn.y && y <= btn.y + btn.height;
}

bool isMouseInInputBox(int x, int y, const Inputbox& box) {
    return x >= box.x && x <= box.x + box.width &&
        y >= box.y && y <= box.y + box.height;
}

void drawInputBox(const Inputbox& box, const wchar_t* text, bool isActive) {
    // 绘制边框
    setlinecolor(box.isHovered ? RGB(173, 216, 230) : box.borderColor);
    setfillcolor(box.bgColor);
    fillrectangle(box.x, box.y, box.x + box.width, box.y + box.height);

    // 设置文本属性
    setbkmode(TRANSPARENT);
    settextstyle(16, 0, _T("宋体"));

    RECT rect = { box.x + 5, box.y + 5, box.x + box.width - 5, box.y + box.height - 5 };

    if (wcslen(text) == 0 && !isActive) {
        // 没有输入且不活跃时显示placeholder
        settextcolor(RGB(150, 150, 150));
        drawtext(box.placeholder, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }
    else {
        // 有输入或活跃时显示内容
        settextcolor(RGB(50, 50, 50));
        if (box.isPassword) {
            wchar_t masked[64] = { 0 };
            for (size_t i = 0; i < wcslen(text); i++) {
                masked[i] = L'*';
            }
            drawtext(masked, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
        else {
            drawtext(text, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
    }

    // 绘制光标（当输入框活跃时）
    if (isActive) {
        int textWidth;
        if (box.isPassword) {
            wchar_t stars[64] = { 0 };
            for (size_t i = 0; i < wcslen(text); i++) {
                stars[i] = L'*';
            }
            textWidth = textwidth(stars);
        }
        else {
            textWidth = textwidth(text);
        }

        int cursorX = box.x + 5 + textWidth;
        setlinecolor(RGB(0, 0, 0));
        line(cursorX, box.y + 5, cursorX, box.y + box.height - 5);
    }
}

bool checkLogin(const wchar_t* account, const wchar_t* password) {
    wifstream inFile("users.txt");
    if (!inFile.is_open()) {
        MessageBox(NULL, _T("用户数据文件不存在"), _T("错误"), MB_OK);
        return false;
    }

    wstring line;
    while (getline(inFile, line)) {
        size_t pos = line.find(L':');
        if (pos != wstring::npos) {
            wstring storedAccount = line.substr(0, pos);
            wstring storedPassword = line.substr(pos + 1);

            if (storedAccount == account && storedPassword == password) {
                inFile.close();
                return true;
            }
        }
    }

    inFile.close();
    return false;
}

// 状态类实现
void LostState::handle(GameManager& manager)
{
    manager.getGame().draw();
    manager.drawTimer();
    FlushBatchDraw();
    wchar_t message[100];
    swprintf_s(message, L"你输了!\n再来一次？");

    int select = MessageBox(NULL, message, L"继续加油吧", MB_OKCANCEL | MB_ICONWARNING);
    if (select == IDOK) {
        manager.getGame().init();
        manager.resetTimer(); // 重置计时器
        manager.startTimer(); // 重新开始游戏时重置计时器
    }
    else {
        exit(0);
    }
}

void WonState::handle(GameManager& manager)
{
    manager.stopTimer(); // 暂停计时器但不重置
    double elapsed = manager.getElapsedTime();
    manager.updateBestTime();
    wchar_t message[100];
    swprintf_s(message, L"恭喜你赢了! 本次用时：%.1f秒\n再来一次？", elapsed);

    int select = MessageBox(NULL, message, L"游戏胜利", MB_OKCANCEL | MB_ICONWARNING);
    if (select == IDOK) {
        manager.restartGame();
    }
    else {
        exit(0);
    }
}

int main()
{
    shouye();
    return 0;
}

// 首页操作
void shouye() {
    // 初始化窗口
    initgraph(600, 600);

    // 启用双缓冲
    BeginBatchDraw();

    // 游戏标题
    settextcolor(WHITE);
    settextstyle(48, 0, _T("黑体"));
    outtextxy(150, 50, _T("扫雷游戏"));

    // 选择提示
    settextstyle(24, 0, _T("宋体"));
    outtextxy(150, 130, _T("请选择:"));

    // 定义按钮
    Button zhuceBtn = { 150, 180, 300, 50, L"注册", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button dengluBtn = { 150, 250, 300, 50, L"登录", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button youshuoBtn = { 150, 320, 300, 50, L"游戏说明", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button caoshuoBtn = { 150, 390, 300, 50, L"操作说明", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button paihangBtn = { 150, 460, 300, 50, L"排行榜", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button opmodeBtn = { 150, 390, 300, 50, L"选择操作模式", RGB(173, 216, 230), RGB(175, 238, 238), false };

    int choice = 0;

    while (true) {
        // 绘制界面
        cleardevice();
        setfillcolor(RGB(255, 193, 193));
        solidrectangle(0, 0, 600, 600);

        // 绘制文字
        settextcolor(WHITE);
        settextstyle(50, 0, _T("黑体"));
        outtextxy(150, 50, _T("扫雷游戏"));
        settextstyle(24, 0, _T("宋体"));
        outtextxy(150, 130, _T("请选择:"));

        // 绘制按钮
        drawButton(zhuceBtn);
        drawButton(dengluBtn);
        drawButton(youshuoBtn);
        drawButton(caoshuoBtn);
        drawButton(paihangBtn);
        drawButton(opmodeBtn);

        // 刷新显示
        FlushBatchDraw();

        // 获取鼠标消息
        MOUSEMSG msg;
        if (MouseHit()) {
            msg = GetMouseMsg();

            // 检查鼠标悬停状态
            zhuceBtn.isHovered = isMouseInButton(msg.x, msg.y, zhuceBtn);
            dengluBtn.isHovered = isMouseInButton(msg.x, msg.y, dengluBtn);
            youshuoBtn.isHovered = isMouseInButton(msg.x, msg.y, youshuoBtn);
            caoshuoBtn.isHovered = isMouseInButton(msg.x, msg.y, caoshuoBtn);
            paihangBtn.isHovered = isMouseInButton(msg.x, msg.y, paihangBtn);
            opmodeBtn.isHovered = isMouseInButton(msg.x, msg.y, opmodeBtn);

            // 检查点击
            if (msg.uMsg == WM_LBUTTONDOWN) {
                if (zhuceBtn.isHovered) {
                    choice = 1;
                    break;
                }
                else if (dengluBtn.isHovered) {
                    choice = 2;
                    break;
                }
                else if (youshuoBtn.isHovered) {
                    choice = 3;
                    break;
                }
                else if (caoshuoBtn.isHovered) {
                    choice = 4;
                    break;
                }
                else if (paihangBtn.isHovered) {
                    choice = 5;
                    break;
                }
                else if (opmodeBtn.isHovered) {
                    showOperationModeSelection();
                    break;
                }
            }
        }

        // 延时，减少CPU占用
        Sleep(10);
    }

    // 关闭图形窗口
    EndBatchDraw();
    closegraph();

    // 根据选择执行相应操作
    switch (choice) {
    case 1:
        zhuce();
        break;
    case 2:
        denglu();
        break;
    case 3:
        showGameInstructions();
        break;
    case 4:
        showGameControls();
        break;
    case 5:
        showRankingDifficultySelection();
        break;
    }
}

void zhuce() {
    // 初始化窗口
    initgraph(600, 600);
    BeginBatchDraw();

    // 背景色
    setfillcolor(RGB(255, 193, 193));
    solidrectangle(0, 0, 600, 600);

    // 标题
    settextcolor(WHITE);
    settextstyle(48, 0, _T("黑体"));
    outtextxy(150, 50, _T("用户注册"));

    // 定义输入框
    Inputbox accountBox = { 150, 180, 300, 40, L"请输入账号",
                          RGB(255, 255, 255), RGB(173, 216, 230), false, false };
    Inputbox passwordBox = { 150, 250, 300, 40, L"请输入密码",
                           RGB(255, 255, 255), RGB(173, 216, 230), false, true };

    // 定义按钮
    Button confirmBtn = { 150, 320, 300, 50, L"确认注册", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button returnBtn = { 150, 390, 300, 50, L"返回首页", RGB(173, 216, 230), RGB(175, 238, 238), false };

    // 输入文本缓冲区
    wchar_t accountText[64] = { 0 };
    wchar_t passwordText[64] = { 0 };
    bool isInputAccount = false;
    bool isInputPassword = false;

    // 主循环
    while (true) {
        // 绘制界面
        cleardevice();
        setfillcolor(RGB(255, 193, 193));
        solidrectangle(0, 0, 600, 600);

        // 绘制标题
        settextcolor(WHITE);
        settextstyle(48, 0, _T("黑体"));
        outtextxy(150, 50, _T("用户注册"));

        // 绘制输入框
        drawInputBox(accountBox, accountText, isInputAccount);
        drawInputBox(passwordBox, passwordText, isInputPassword);

        // 绘制按钮
        drawButton(confirmBtn);
        drawButton(returnBtn);

        // 刷新显示
        FlushBatchDraw();

        // 处理鼠标和键盘消息
        MOUSEMSG msg;
        while (MouseHit()) {
            msg = GetMouseMsg();

            // 更新悬停状态
            accountBox.isHovered = isMouseInInputBox(msg.x, msg.y, accountBox);
            passwordBox.isHovered = isMouseInInputBox(msg.x, msg.y, passwordBox);
            confirmBtn.isHovered = isMouseInButton(msg.x, msg.y, confirmBtn);
            returnBtn.isHovered = isMouseInButton(msg.x, msg.y, returnBtn);

            // 处理点击
            if (msg.uMsg == WM_LBUTTONDOWN) {
                if (accountBox.isHovered) {
                    isInputAccount = true;
                    isInputPassword = false;
                }
                else if (passwordBox.isHovered) {
                    isInputAccount = false;
                    isInputPassword = true;
                }
                else if (confirmBtn.isHovered) {
                    // **检查账号是否已存在**
                    if (wcslen(accountText) == 0 || wcslen(passwordText) == 0) {
                        MessageBox(NULL, _T("账号和密码不能为空"), _T("提示"), MB_OK);
                    }
                    else {
                        // **读取 users.txt 文件，检查是否已存在该账号**
                        bool isAccountExist = false;
                        wifstream inFile("users.txt");
                        if (inFile.is_open()) {
                            wstring line;
                            while (getline(inFile, line)) {
                                size_t pos = line.find(L':');
                                if (pos != wstring::npos) {
                                    wstring storedAccount = line.substr(0, pos);
                                    if (storedAccount == accountText) {
                                        isAccountExist = true;
                                        break;
                                    }
                                }
                            }
                            inFile.close();
                        }

                        if (isAccountExist) {
                            MessageBox(NULL, _T("该账号已被注册"), _T("提示"), MB_OK);
                        }
                        else {
                            // **写入新账号**
                            wofstream outFile("users.txt", ios::app);
                            if (outFile.is_open()) {
                                outFile << accountText << L":" << passwordText << endl;
                                outFile.close();
                                MessageBox(NULL, _T("注册成功"), _T("提示"), MB_OK);
                                EndBatchDraw();
                                closegraph();
                                shouye();
                                return;
                            }
                            else {
                                MessageBox(NULL, _T("无法打开文件"), _T("错误"), MB_OK);
                            }
                        }
                    }
                }
                else if (returnBtn.isHovered) {
                    EndBatchDraw();
                    closegraph();
                    shouye();
                    return;
                }
                else {
                    // 点击了其他地方，取消输入状态
                    isInputAccount = false;
                    isInputPassword = false;
                }
            }
        }

        // 处理键盘输入（关键修改部分）
        if (isInputAccount || isInputPassword) {
            // 检查退格键
            if (GetAsyncKeyState(VK_BACK) & 0x8000) {
                if (isInputAccount && wcslen(accountText) > 0) {
                    accountText[wcslen(accountText) - 1] = L'\0';
                    Sleep(100); // 防止连续删除
                }
                else if (isInputPassword && wcslen(passwordText) > 0) {
                    passwordText[wcslen(passwordText) - 1] = L'\0';
                    Sleep(100);
                }
            }
            // 检查Tab键切换
            else if (GetAsyncKeyState(VK_TAB) & 0x8000) {
                isInputAccount = !isInputAccount;
                isInputPassword = !isInputPassword;
                Sleep(100);
            }
            // 检查普通字符输入
            else {
                for (int ch = 0x20; ch <= 0x7E; ch++) { // ASCII可打印字符
                    if (GetAsyncKeyState(ch) & 0x8000) {
                        if (isInputAccount && wcslen(accountText) < 63) {
                            size_t len = wcslen(accountText);
                            accountText[len] = (wchar_t)ch;
                            accountText[len + 1] = L'\0';
                            Sleep(100); // 防止重复输入
                        }
                        else if (isInputPassword && wcslen(passwordText) < 63) {
                            size_t len = wcslen(passwordText);
                            passwordText[len] = (wchar_t)ch;
                            passwordText[len + 1] = L'\0';
                            Sleep(100);
                        }
                    }
                }
            }
        }

        // 延时
        Sleep(10);
    }
}

void denglu() {
    // 初始化窗口
    initgraph(600, 600);
    BeginBatchDraw();

    // 背景色
    setfillcolor(RGB(255, 193, 193));
    solidrectangle(0, 0, 600, 600);

    // 标题
    settextcolor(WHITE);
    settextstyle(48, 0, _T("黑体"));
    outtextxy(150, 50, _T("用户登录"));

    // 定义输入框
    Inputbox accountBox = { 150, 180, 300, 40, L"请输入账号",
                          RGB(255, 255, 255), RGB(173, 216, 230), false, false };
    Inputbox passwordBox = { 150, 250, 300, 40, L"请输入密码",
                           RGB(255, 255, 255), RGB(173, 216, 230), false, true };

    // 定义按钮
    Button loginBtn = { 150, 320, 300, 50, L"登录", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button returnBtn = { 150, 390, 300, 50, L"返回首页", RGB(173, 216, 230), RGB(175, 238, 238), false };

    // 输入文本缓冲区
    wchar_t accountText[64] = { 0 };
    wchar_t passwordText[64] = { 0 };
    bool isInputAccount = false;
    bool isInputPassword = false;

    // 主循环
    while (true) {
        // 绘制界面
        cleardevice();
        setfillcolor(RGB(255, 193, 193));
        solidrectangle(0, 0, 600, 600);

        // 绘制标题
        settextcolor(WHITE);
        settextstyle(48, 0, _T("黑体"));
        outtextxy(150, 50, _T("用户登录"));

        // 绘制输入框
        drawInputBox(accountBox, accountText, isInputAccount);
        drawInputBox(passwordBox, passwordText, isInputPassword);

        // 绘制按钮
        drawButton(loginBtn);
        drawButton(returnBtn);

        // 刷新显示
        FlushBatchDraw();

        // 处理鼠标和键盘消息
        MOUSEMSG msg;
        while (MouseHit()) {
            msg = GetMouseMsg();

            // 更新悬停状态
            accountBox.isHovered = isMouseInInputBox(msg.x, msg.y, accountBox);
            passwordBox.isHovered = isMouseInInputBox(msg.x, msg.y, passwordBox);
            loginBtn.isHovered = isMouseInButton(msg.x, msg.y, loginBtn);
            returnBtn.isHovered = isMouseInButton(msg.x, msg.y, returnBtn);
            // 处理点击
            if (msg.uMsg == WM_LBUTTONDOWN) {
                if (accountBox.isHovered) {
                    isInputAccount = true;
                    isInputPassword = false;
                }
                else if (passwordBox.isHovered) {
                    isInputAccount = false;
                    isInputPassword = true;
                }
                else if (loginBtn.isHovered) {
                    // 登录逻辑
                    if (wcslen(accountText) == 0 || wcslen(passwordText) == 0) {
                        MessageBox(NULL, _T("账号和密码不能为空"), _T("提示"), MB_OK);
                    }
                    else {
                        if (checkLogin(accountText, passwordText)) {
                            MessageBox(NULL, _T("登录成功！"), _T("提示"), MB_OK);
                            // 登录成功后可以跳转到游戏界面
                            EndBatchDraw();
                            closegraph();

                            int Rows = 10, Cols = 10, mineNum = 15;

                            // 显示难度选择对话框
                            showDifficultyDialog(Rows, Cols, mineNum);

                            GameManager gameManager(Rows, Cols, mineNum, accountText);
                            gameManager.run();
                            delete ResourceManager::getInstance(); // 新增

                            // 这里可以调用游戏主界面函数
                            return;
                        }
                        else {
                            MessageBox(NULL, _T("账号或密码错误"), _T("提示"), MB_OK);
                        }
                    }
                }
                else if (returnBtn.isHovered) {
                    EndBatchDraw();
                    closegraph();
                    shouye();
                    return;
                }
                else {
                    // 点击了其他地方，取消输入状态
                    isInputAccount = false;
                    isInputPassword = false;
                }
            }
        }
        // 处理键盘输入（关键修改部分）
        if (isInputAccount || isInputPassword) {
            // 检查退格键
            if (GetAsyncKeyState(VK_BACK) & 0x8000) {
                if (isInputAccount && wcslen(accountText) > 0) {
                    accountText[wcslen(accountText) - 1] = L'\0';
                    Sleep(100); // 防止连续删除
                }
                else if (isInputPassword && wcslen(passwordText) > 0) {
                    passwordText[wcslen(passwordText) - 1] = L'\0';
                    Sleep(100);
                }
            }
            // 检查Tab键切换
            else if (GetAsyncKeyState(VK_TAB) & 0x8000) {
                isInputAccount = !isInputAccount;
                isInputPassword = !isInputPassword;
                Sleep(100);
            }
            // 检查普通字符输入
            else {
                for (int ch = 0x20; ch <= 0x7E; ch++) { // ASCII可打印字符
                    if (GetAsyncKeyState(ch) & 0x8000) {
                        if (isInputAccount && wcslen(accountText) < 63) {
                            size_t len = wcslen(accountText);
                            accountText[len] = (wchar_t)ch;
                            accountText[len + 1] = L'\0';
                            Sleep(100); // 防止重复输入
                        }
                        else if (isInputPassword && wcslen(passwordText) < 63) {
                            size_t len = wcslen(passwordText);
                            passwordText[len] = (wchar_t)ch;
                            passwordText[len + 1] = L'\0';
                            Sleep(100);
                        }
                    }
                }
            }
        }

        // 延时
        Sleep(10);
    }
}

// 显示操作模式选择对话框
void showOperationModeSelection() {
    initgraph(600, 600);
    BeginBatchDraw();

    Button mouseBtn = { 150, 180, 300, 50, L"默认（鼠标操作）", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button keyboardBtn = { 150, 250, 300, 50, L"自定义（键盘操作）", RGB(173, 216, 230), RGB(175, 238, 238), false };
    Button backBtn = { 150, 320, 300, 50, L"返回", RGB(200, 200, 200), RGB(220, 220, 220), false };

    while (true) {
        cleardevice();
        setfillcolor(RGB(255, 193, 193));
        solidrectangle(0, 0, 600, 600);

        settextcolor(WHITE);
        settextstyle(48, 0, _T("黑体"));
        outtextxy(150, 50, _T("操作模式选择"));

        drawButton(mouseBtn);
        drawButton(keyboardBtn);
        drawButton(backBtn);

        FlushBatchDraw();

        MOUSEMSG msg;
        if (MouseHit()) {
            msg = GetMouseMsg();

            mouseBtn.isHovered = isMouseInButton(msg.x, msg.y, mouseBtn);
            keyboardBtn.isHovered = isMouseInButton(msg.x, msg.y, keyboardBtn);
            backBtn.isHovered = isMouseInButton(msg.x, msg.y, backBtn);

            if (msg.uMsg == WM_LBUTTONDOWN) {
                if (mouseBtn.isHovered) {
                    currentOperationMode = OperationMode::Mouse; // 设置为鼠标模式
                    EndBatchDraw();
                    closegraph();
                    shouye();
                    return;
                }
                else if (keyboardBtn.isHovered) {
                    currentOperationMode = OperationMode::Keyboard; // 设置为键盘模式
                    EndBatchDraw();
                    closegraph();
                    shouye();
                    return;
                }
                else if (backBtn.isHovered) {
                    EndBatchDraw();
                    closegraph();
                    shouye();
                    return;
                }
            }
        }
        Sleep(10);
    }
}