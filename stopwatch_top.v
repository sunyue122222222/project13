/*
 * FPGA数码管秒表计时器顶层模块
 * 功能：
 * - KEY0: 正计时控制（1=开始，0=暂停）
 * - KEY1: 倒计时控制（1=开始，0=暂停）
 * - KEY2: 模式切换（正计时/倒计时）
 * - KEY4: 倒计时初始值设置模式
 * - KEY5: 频率调整（1Hz->2Hz->4Hz->1Hz）
 * - KEY6: 显示精度调整（1秒<->0.1秒）
 * - 复位键: 恢复初始状态
 */

module stopwatch_top(
    input wire clk,           // 系统时钟 50MHz
    input wire rst_n,         // 复位信号（低有效）
    input wire [6:0] key,     // 按键输入 KEY[6:0]
    output wire [7:0] seg,    // 数码管段选信号
    output wire [5:0] dig,    // 数码管位选信号（6位数码管）
    output wire [7:0] led     // LED指示灯
);

// 内部信号定义
wire clk_1hz, clk_2hz, clk_4hz;
wire clk_display;
wire [6:0] key_pulse;
wire [6:0] key_stable;

// 计时器相关信号
reg [1:0] freq_mode;          // 频率模式：00=1Hz, 01=2Hz, 10=4Hz
reg precision_mode;           // 精度模式：0=1秒, 1=0.1秒
reg timer_mode;               // 计时模式：0=正计时, 1=倒计时
reg setting_mode;             // 设置模式：0=正常, 1=设置倒计时初始值

reg [15:0] count_up;          // 正计时计数器（秒*10）
reg [15:0] count_down;        // 倒计时计数器（秒*10）
reg [15:0] initial_value;     // 倒计时初始值（秒*10）
reg [15:0] display_value;     // 显示值

reg count_up_enable;
reg count_down_enable;
reg led_blink_enable;

// 时钟分频模块
clock_divider clk_div(
    .clk(clk),
    .rst_n(rst_n),
    .clk_1hz(clk_1hz),
    .clk_2hz(clk_2hz),
    .clk_4hz(clk_4hz)
);

// 按键消抖模块
genvar i;
generate
    for (i = 0; i < 7; i = i + 1) begin : key_debounce_gen
        key_debounce key_db(
            .clk(clk),
            .rst_n(rst_n),
            .key_in(key[i]),
            .key_out(key_stable[i]),
            .key_pulse(key_pulse[i])
        );
    end
endgenerate

// 数码管显示模块
seg_display seg_disp(
    .clk(clk),
    .rst_n(rst_n),
    .value(display_value),
    .precision_mode(precision_mode),
    .seg(seg),
    .dig(dig)
);

// 选择工作时钟
assign clk_display = (freq_mode == 2'b00) ? clk_1hz :
                    (freq_mode == 2'b01) ? clk_2hz : clk_4hz;

// 主控制逻辑
always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        freq_mode <= 2'b00;
        precision_mode <= 1'b0;
        timer_mode <= 1'b0;
        setting_mode <= 1'b0;
        count_up <= 16'h0000;
        count_down <= 16'd190;  // 19.0秒
        initial_value <= 16'd190;
        display_value <= 16'h0000;
        count_up_enable <= 1'b0;
        count_down_enable <= 1'b0;
        led_blink_enable <= 1'b0;
    end else begin
        // KEY5: 频率调整
        if (key_pulse[5]) begin
            freq_mode <= (freq_mode == 2'b10) ? 2'b00 : freq_mode + 1'b1;
        end
        
        // KEY6: 精度调整
        if (key_pulse[6]) begin
            precision_mode <= ~precision_mode;
        end
        
        // KEY2: 模式切换
        if (key_pulse[2]) begin
            timer_mode <= ~timer_mode;
            count_up_enable <= 1'b0;
            count_down_enable <= 1'b0;
        end
        
        // KEY4: 设置模式切换
        if (key_pulse[4]) begin
            if (!setting_mode) begin
                setting_mode <= 1'b1;
                count_down <= initial_value;
            end else begin
                setting_mode <= 1'b0;
                count_down_enable <= 1'b1;
            end
        end
        
        // 设置模式下的初始值调整
        if (setting_mode) begin
            if (key_pulse[0]) begin  // KEY0在设置模式下增加初始值
                if (initial_value < 16'd9990) // 最大999.0秒
                    initial_value <= initial_value + (precision_mode ? 16'd1 : 16'd10);
            end
            if (key_pulse[1]) begin  // KEY1在设置模式下减少初始值
                if (initial_value > 16'd100)  // 最小10.0秒
                    initial_value <= initial_value - (precision_mode ? 16'd1 : 16'd10);
            end
            count_down <= initial_value;
        end else begin
            // 正常计时模式
            if (!timer_mode) begin  // 正计时模式
                count_up_enable <= key_stable[0];
                count_down_enable <= 1'b0;
                if (key_pulse[1]) begin  // KEY1复位正计时
                    count_up <= 16'h0000;
                end
            end else begin  // 倒计时模式
                count_up_enable <= 1'b0;
                count_down_enable <= key_stable[1];
                if (key_pulse[0]) begin  // KEY0复位倒计时
                    count_down <= initial_value;
                end
            end
        end
        
        // 显示值选择
        if (setting_mode) begin
            display_value <= initial_value;
        end else if (!timer_mode) begin
            display_value <= count_up;
        end else begin
            display_value <= count_down;
        end
        
        // LED闪烁控制（倒计时最后3秒）
        led_blink_enable <= (timer_mode && count_down <= 16'd30 && count_down > 16'd0);
    end
end

// 计时器更新逻辑
always @(posedge clk_display or negedge rst_n) begin
    if (!rst_n) begin
        count_up <= 16'h0000;
        count_down <= 16'd190;
    end else begin
        if (!setting_mode) begin
            // 正计时
            if (count_up_enable && count_up < 16'd9999) begin
                count_up <= count_up + 1'b1;
            end
            
            // 倒计时
            if (count_down_enable && count_down > 16'd0) begin
                count_down <= count_down - 1'b1;
            end
        end
    end
end

// LED控制
led_controller led_ctrl(
    .clk(clk),
    .rst_n(rst_n),
    .blink_enable(led_blink_enable),
    .timer_mode(timer_mode),
    .setting_mode(setting_mode),
    .freq_mode(freq_mode),
    .precision_mode(precision_mode),
    .led(led)
);

endmodule