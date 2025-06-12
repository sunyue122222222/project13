/*
 * LED控制模块
 * 控制LED指示灯显示系统状态和闪烁提示
 */

module led_controller(
    input wire clk,              // 系统时钟
    input wire rst_n,            // 复位信号
    input wire blink_enable,     // 闪烁使能（倒计时最后3秒）
    input wire timer_mode,       // 计时模式：0=正计时，1=倒计时
    input wire setting_mode,     // 设置模式
    input wire [1:0] freq_mode,  // 频率模式
    input wire precision_mode,   // 精度模式
    output reg [7:0] led         // LED输出
);

// 闪烁时钟分频
parameter BLINK_FREQ = 24'd12_500_000;  // 2Hz闪烁频率 @ 50MHz
reg [23:0] blink_cnt;
reg blink_clk;

// 闪烁时钟生成
always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        blink_cnt <= 24'd0;
        blink_clk <= 1'b0;
    end else begin
        if (blink_cnt >= BLINK_FREQ - 1) begin
            blink_cnt <= 24'd0;
            blink_clk <= ~blink_clk;
        end else begin
            blink_cnt <= blink_cnt + 1'b1;
        end
    end
end

// LED控制逻辑
always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        led <= 8'b00000000;
    end else begin
        // 默认状态
        led <= 8'b00000000;
        
        // LED0-1: 频率模式指示
        case (freq_mode)
            2'b00: led[1:0] <= 2'b01;  // 1Hz
            2'b01: led[1:0] <= 2'b10;  // 2Hz
            2'b10: led[1:0] <= 2'b11;  // 4Hz
            default: led[1:0] <= 2'b00;
        endcase
        
        // LED2: 精度模式指示
        led[2] <= precision_mode;
        
        // LED3: 计时模式指示
        led[3] <= timer_mode;
        
        // LED4: 设置模式指示
        if (setting_mode) begin
            led[4] <= blink_clk;  // 设置模式时闪烁
        end else begin
            led[4] <= 1'b0;
        end
        
        // LED5-7: 倒计时警告闪烁
        if (blink_enable) begin
            led[7:5] <= {3{blink_clk}};
        end else begin
            led[7:5] <= 3'b000;
        end
    end
end

endmodule