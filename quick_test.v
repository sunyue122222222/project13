/*
 * 快速功能验证测试平台
 * 适用于Vivado快速仿真验证
 * 测试时间较短，重点验证核心功能
 */

`timescale 1ns / 1ps

module quick_test;

    // 信号声明
    reg clk;
    reg rst_n;
    reg [6:0] key;
    wire [7:0] seg;
    wire [5:0] dig;
    wire [7:0] led;

    // 实例化被测设计
    stopwatch_top uut (
        .clk(clk),
        .rst_n(rst_n),
        .key(key),
        .seg(seg),
        .dig(dig),
        .led(led)
    );

    // 时钟生成 - 50MHz
    initial begin
        clk = 0;
        forever #10 clk = ~clk;
    end

    // 按键操作任务
    task press_key;
        input [2:0] key_num;
        begin
            key[key_num] = 1'b0;
            #100000;  // 100us按键持续时间
            key[key_num] = 1'b1;
            #100000;  // 100us等待时间
        end
    endtask

    // 主测试序列
    initial begin
        // 初始化
        rst_n = 0;
        key = 7'b1111111;
        
        $display("=== 快速功能测试开始 ===");
        
        // 复位
        #200000;
        rst_n = 1;
        #100000;
        
        $display("时间 %t: 系统复位完成", $time);
        $display("初始状态 - 模式:%s, 显示值:%d, LED:%b", 
                uut.timer_mode ? "倒计时" : "正计时", 
                uut.display_value, led);
        
        // 测试1: 正计时
        $display("\n--- 测试正计时功能 ---");
        press_key(0);  // KEY0开始正计时
        $display("按下KEY0，开始正计时");
        
        #2000000;  // 等待2ms观察计时
        $display("正计时值: %d", uut.count_up);
        
        press_key(0);  // 暂停
        $display("按下KEY0，暂停计时");
        
        // 测试2: 切换到倒计时
        $display("\n--- 测试倒计时功能 ---");
        press_key(2);  // KEY2切换模式
        $display("按下KEY2，切换到倒计时模式");
        $display("倒计时初始值: %d", uut.count_down);
        
        press_key(1);  // KEY1开始倒计时
        $display("按下KEY1，开始倒计时");
        
        #3000000;  // 等待3ms观察倒计时
        $display("倒计时值: %d", uut.count_down);
        
        // 测试3: 频率调整
        $display("\n--- 测试频率调整 ---");
        press_key(5);  // KEY5调整频率
        $display("按下KEY5，频率模式: %d", uut.freq_mode);
        
        press_key(5);  // 再次调整
        $display("再次按下KEY5，频率模式: %d", uut.freq_mode);
        
        // 测试4: 精度调整
        $display("\n--- 测试精度调整 ---");
        press_key(6);  // KEY6调整精度
        $display("按下KEY6，精度模式: %d", uut.precision_mode);
        
        // 测试5: 设置模式
        $display("\n--- 测试设置模式 ---");
        press_key(4);  // KEY4进入设置模式
        $display("按下KEY4，进入设置模式: %d", uut.setting_mode);
        $display("当前初始值: %d", uut.initial_value);
        
        press_key(0);  // 增加初始值
        $display("按下KEY0增加初始值: %d", uut.initial_value);
        
        press_key(4);  // 退出设置模式
        $display("按下KEY4，退出设置模式");
        
        #2000000;  // 观察设置后的倒计时
        $display("设置后倒计时值: %d", uut.count_down);
        
        // 显示最终状态
        $display("\n=== 测试完成 ===");
        $display("最终状态:");
        $display("- 计时模式: %s", uut.timer_mode ? "倒计时" : "正计时");
        $display("- 精度模式: %s", uut.precision_mode ? "0.1秒" : "1秒");
        $display("- 频率模式: %d", uut.freq_mode);
        $display("- 正计时值: %d", uut.count_up);
        $display("- 倒计时值: %d", uut.count_down);
        $display("- 显示值: %d", uut.display_value);
        $display("- LED状态: %b", led);
        
        #1000000;  // 额外等待时间
        $finish;
    end

    // 监控重要信号变化
    always @(posedge clk) begin
        if (uut.key_pulse != 7'b0000000) begin
            $display("时间 %t: 按键脉冲检测 %b", $time, uut.key_pulse);
        end
    end
    
    // 监控计时器更新
    always @(posedge uut.clk_display) begin
        if (uut.count_up_enable) begin
            $display("时间 %t: 正计时更新 = %d", $time, uut.count_up);
        end
        if (uut.count_down_enable) begin
            $display("时间 %t: 倒计时更新 = %d", $time, uut.count_down);
        end
    end

endmodule