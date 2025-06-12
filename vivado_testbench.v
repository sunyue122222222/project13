/*
 * Vivado专用仿真测试平台
 * 适用于FPGA秒表计时器完整功能验证
 * 包含详细的测试序列和波形观察
 */

`timescale 1ns / 1ps

module vivado_testbench;

    // 测试信号声明
    reg clk;
    reg rst_n;
    reg [6:0] key;
    wire [7:0] seg;
    wire [5:0] dig;
    wire [7:0] led;
    
    // 内部信号监控
    wire [1:0] freq_mode;
    wire precision_mode;
    wire timer_mode;
    wire setting_mode;
    wire [15:0] count_up;
    wire [15:0] count_down;
    wire [15:0] display_value;
    wire [15:0] initial_value;
    
    // 时钟信号监控
    wire clk_1hz, clk_2hz, clk_4hz;
    
    // 按键状态监控
    wire [6:0] key_pulse;
    wire [6:0] key_stable;

    // 实例化被测设计
    stopwatch_top uut (
        .clk(clk),
        .rst_n(rst_n),
        .key(key),
        .seg(seg),
        .dig(dig),
        .led(led)
    );
    
    // 连接内部信号用于监控
    assign freq_mode = uut.freq_mode;
    assign precision_mode = uut.precision_mode;
    assign timer_mode = uut.timer_mode;
    assign setting_mode = uut.setting_mode;
    assign count_up = uut.count_up;
    assign count_down = uut.count_down;
    assign display_value = uut.display_value;
    assign initial_value = uut.initial_value;
    
    assign clk_1hz = uut.clk_1hz;
    assign clk_2hz = uut.clk_2hz;
    assign clk_4hz = uut.clk_4hz;
    
    assign key_pulse = uut.key_pulse;
    assign key_stable = uut.key_stable;

    // 时钟生成 - 50MHz
    initial begin
        clk = 0;
        forever #10 clk = ~clk;  // 20ns周期 = 50MHz
    end

    // 测试任务定义
    task press_key;
        input [2:0] key_num;
        input integer duration;
        begin
            $display("时间 %t: 按下 KEY%0d", $time, key_num);
            key[key_num] = 1'b0;  // 按键按下（低有效）
            #(duration * 1000);   // 持续时间（微秒转纳秒）
            key[key_num] = 1'b1;  // 按键释放
            #50000;               // 等待消抖完成
        end
    endtask
    
    task wait_cycles;
        input integer cycles;
        begin
            repeat(cycles) @(posedge clk);
        end
    endtask
    
    task wait_ms;
        input integer ms;
        begin
            #(ms * 1000000);  // 毫秒转纳秒
        end
    endtask
    
    // 显示当前状态的任务
    task display_status;
        begin
            $display("=== 系统状态 ===");
            $display("时间: %t", $time);
            $display("模式: %s", timer_mode ? "倒计时" : "正计时");
            $display("精度: %s", precision_mode ? "0.1秒" : "1秒");
            $display("频率: %s", (freq_mode == 2'b00) ? "1Hz" : 
                                (freq_mode == 2'b01) ? "2Hz" : "4Hz");
            $display("设置模式: %s", setting_mode ? "是" : "否");
            $display("正计时值: %d", count_up);
            $display("倒计时值: %d", count_down);
            $display("显示值: %d", display_value);
            $display("初始值: %d", initial_value);
            $display("LED状态: %b", led);
            $display("================");
        end
    endtask

    // 主测试序列
    initial begin
        // 初始化信号
        rst_n = 1'b0;
        key = 7'b1111111;  // 所有按键释放（高有效）
        
        $display("=== Vivado仿真测试开始 ===");
        $display("系统时钟: 50MHz");
        $display("仿真时间单位: 1ns");
        
        // 复位序列
        $display("\n--- 复位测试 ---");
        wait_ms(100);
        rst_n = 1'b1;
        wait_ms(100);
        display_status();
        
        // 测试1: 正计时基本功能
        $display("\n--- 测试1: 正计时功能 ---");
        press_key(0, 50);  // 按KEY0开始正计时
        display_status();
        
        // 等待一段时间观察计时
        wait_ms(500);
        display_status();
        
        press_key(0, 50);  // 再次按KEY0暂停
        display_status();
        
        wait_ms(200);
        press_key(0, 50);  // 继续计时
        display_status();
        
        wait_ms(300);
        press_key(1, 50);  // 按KEY1复位
        display_status();
        
        // 测试2: 切换到倒计时模式
        $display("\n--- 测试2: 倒计时模式 ---");
        press_key(2, 50);  // 按KEY2切换模式
        display_status();
        
        press_key(1, 50);  // 按KEY1开始倒计时
        display_status();
        
        wait_ms(1000);     // 等待观察倒计时
        display_status();
        
        press_key(1, 50);  // 暂停倒计时
        display_status();
        
        press_key(0, 50);  // 复位倒计时
        display_status();
        
        // 测试3: 频率调整功能
        $display("\n--- 测试3: 频率调整 ---");
        press_key(5, 50);  // 调整到2Hz
        display_status();
        
        press_key(1, 50);  // 开始倒计时观察2Hz
        wait_ms(500);
        display_status();
        
        press_key(5, 50);  // 调整到4Hz
        display_status();
        wait_ms(500);
        display_status();
        
        press_key(5, 50);  // 调整回1Hz
        display_status();
        
        press_key(1, 50);  // 暂停
        
        // 测试4: 精度调整功能
        $display("\n--- 测试4: 精度调整 ---");
        press_key(6, 50);  // 切换到0.1秒精度
        display_status();
        
        press_key(1, 50);  // 开始倒计时观察0.1秒精度
        wait_ms(800);
        display_status();
        
        press_key(6, 50);  // 切换回1秒精度
        display_status();
        
        press_key(1, 50);  // 暂停
        
        // 测试5: 设置模式功能
        $display("\n--- 测试5: 设置模式 ---");
        press_key(4, 50);  // 进入设置模式
        display_status();
        
        // 调整初始值
        press_key(0, 50);  // 增加初始值
        display_status();
        press_key(0, 50);  // 再次增加
        display_status();
        press_key(1, 50);  // 减少初始值
        display_status();
        
        press_key(4, 50);  // 退出设置模式并开始倒计时
        display_status();
        
        wait_ms(1000);     // 观察自定义初始值的倒计时
        display_status();
        
        // 测试6: LED警告功能（倒计时最后几秒）
        $display("\n--- 测试6: LED警告功能 ---");
        // 设置一个较小的初始值来快速测试警告
        press_key(4, 50);  // 进入设置模式
        
        // 设置初始值为约3秒（30个0.1秒单位）
        repeat(10) begin
            press_key(1, 30);  // 快速减少初始值
        end
        display_status();
        
        press_key(4, 50);  // 退出设置并开始倒计时
        display_status();
        
        // 观察LED警告
        wait_ms(2000);
        display_status();
        
        // 测试7: 模式切换回正计时
        $display("\n--- 测试7: 切换回正计时 ---");
        press_key(2, 50);  // 切换到正计时模式
        display_status();
        
        press_key(0, 50);  // 开始正计时
        wait_ms(800);
        display_status();
        
        // 测试8: 综合功能测试
        $display("\n--- 测试8: 综合功能测试 ---");
        press_key(6, 50);  // 0.1秒精度
        press_key(5, 50);  // 2Hz频率
        display_status();
        
        wait_ms(1000);
        display_status();
        
        $display("\n=== 所有测试完成 ===");
        $display("仿真总时间: %t", $time);
        
        // 额外等待时间以观察波形
        wait_ms(500);
        
        $finish;
    end

    // 连续监控重要信号变化
    always @(posedge clk) begin
        // 监控按键脉冲
        if (|key_pulse) begin
            $display("时间 %t: 检测到按键脉冲 %b", $time, key_pulse);
        end
        
        // 监控模式变化
        if ($past(timer_mode) !== timer_mode) begin
            $display("时间 %t: 计时模式变化为 %s", $time, timer_mode ? "倒计时" : "正计时");
        end
        
        // 监控精度变化
        if ($past(precision_mode) !== precision_mode) begin
            $display("时间 %t: 精度模式变化为 %s", $time, precision_mode ? "0.1秒" : "1秒");
        end
        
        // 监控频率变化
        if ($past(freq_mode) !== freq_mode) begin
            $display("时间 %t: 频率模式变化为 %s", $time, 
                    (freq_mode == 2'b00) ? "1Hz" : 
                    (freq_mode == 2'b01) ? "2Hz" : "4Hz");
        end
    end
    
    // 监控计时器变化
    always @(posedge uut.clk_display) begin
        if (!setting_mode) begin
            if (!timer_mode && uut.count_up_enable) begin
                $display("时间 %t: 正计时更新 = %d", $time, count_up);
            end
            if (timer_mode && uut.count_down_enable) begin
                $display("时间 %t: 倒计时更新 = %d", $time, count_down);
                if (count_down <= 30 && count_down > 0) begin
                    $display("时间 %t: *** 警告：倒计时即将结束！***", $time);
                end
            end
        end
    end
    
    // 数码管显示监控
    always @(uut.seg_disp.scan_pos) begin
        if (uut.seg_disp.scan_pos == 0) begin  // 每次扫描周期开始时显示
            $display("时间 %t: 数码管显示值 = %d (格式: %s)", 
                    $time, display_value, precision_mode ? "XXX.X" : "XXXX");
        end
    end

endmodule