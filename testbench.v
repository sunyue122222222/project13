/*
 * 秒表计时器测试平台
 */

`timescale 1ns / 1ps

module testbench;

// 测试信号
reg clk;
reg rst_n;
reg [6:0] key;
wire [7:0] seg;
wire [5:0] dig;
wire [7:0] led;

// 实例化被测模块
stopwatch_top uut (
    .clk(clk),
    .rst_n(rst_n),
    .key(key),
    .seg(seg),
    .dig(dig),
    .led(led)
);

// 时钟生成（50MHz）
initial begin
    clk = 0;
    forever #10 clk = ~clk;  // 20ns周期 = 50MHz
end

// 测试序列
initial begin
    // 初始化
    rst_n = 0;
    key = 7'b1111111;  // 所有按键释放（高电平）
    
    // 复位
    #100;
    rst_n = 1;
    #100;
    
    $display("=== 秒表计时器测试开始 ===");
    
    // 测试1：正计时功能
    $display("测试1：正计时功能");
    key[0] = 0;  // 按下KEY0开始正计时
    #50;
    key[0] = 1;  // 释放KEY0
    #1000000;    // 等待一段时间观察计时
    
    key[0] = 0;  // 再次按下KEY0暂停
    #50;
    key[0] = 1;
    #500000;
    
    // 测试2：切换到倒计时模式
    $display("测试2：倒计时模式");
    key[2] = 0;  // 按下KEY2切换模式
    #50;
    key[2] = 1;
    #100;
    
    key[1] = 0;  // 按下KEY1开始倒计时
    #50;
    key[1] = 1;
    #2000000;   // 等待观察倒计时
    
    // 测试3：频率调整
    $display("测试3：频率调整");
    key[5] = 0;  // 按下KEY5调整频率到2Hz
    #50;
    key[5] = 1;
    #1000000;
    
    key[5] = 0;  // 再次按下KEY5调整频率到4Hz
    #50;
    key[5] = 1;
    #1000000;
    
    // 测试4：精度调整
    $display("测试4：精度调整");
    key[6] = 0;  // 按下KEY6调整精度到0.1秒
    #50;
    key[6] = 1;
    #1000000;
    
    // 测试5：设置模式
    $display("测试5：设置模式");
    key[4] = 0;  // 进入设置模式
    #50;
    key[4] = 1;
    #100;
    
    // 在设置模式下调整初始值
    key[0] = 0;  // 增加初始值
    #50;
    key[0] = 1;
    #100;
    
    key[4] = 0;  // 退出设置模式并开始倒计时
    #50;
    key[4] = 1;
    #2000000;
    
    $display("=== 测试完成 ===");
    $finish;
end

// 监控显示
always @(posedge clk) begin
    if (uut.key_pulse[0] || uut.key_pulse[1] || uut.key_pulse[2] || 
        uut.key_pulse[4] || uut.key_pulse[5] || uut.key_pulse[6]) begin
        $display("时间: %t, 按键脉冲: %b, 模式: %s, 显示值: %d, LED: %b", 
                 $time, uut.key_pulse, 
                 uut.timer_mode ? "倒计时" : "正计时",
                 uut.display_value, led);
    end
end

// 生成波形文件
initial begin
    $dumpfile("stopwatch_tb.vcd");
    $dumpvars(0, testbench);
end

endmodule