/*
 * 数码管显示模块
 * 支持6位数码管显示，可显示秒和0.1秒精度
 * 显示格式：XXX.X（精度模式）或 XXXX（秒模式）
 */

module seg_display(
    input wire clk,              // 系统时钟
    input wire rst_n,            // 复位信号
    input wire [15:0] value,     // 显示值（以0.1秒为单位）
    input wire precision_mode,   // 精度模式：0=秒，1=0.1秒
    output reg [7:0] seg,        // 段选信号（a,b,c,d,e,f,g,dp）
    output reg [5:0] dig         // 位选信号（6位数码管）
);

// 数码管扫描时钟分频
parameter SCAN_FREQ = 16'd1000;  // 扫描频率分频值，约1kHz
reg [15:0] scan_cnt;
reg [2:0] scan_pos;

// BCD码转换
reg [3:0] digit [5:0];  // 6位数字
reg [15:0] bcd_value;
reg [3:0] current_digit;
reg dot_enable;

// 7段数码管编码表（共阴极）
function [7:0] seg_decode;
    input [3:0] digit;
    begin
        case (digit)
            4'h0: seg_decode = 8'b11111100;  // 0
            4'h1: seg_decode = 8'b01100000;  // 1
            4'h2: seg_decode = 8'b11011010;  // 2
            4'h3: seg_decode = 8'b11110010;  // 3
            4'h4: seg_decode = 8'b01100110;  // 4
            4'h5: seg_decode = 8'b10110110;  // 5
            4'h6: seg_decode = 8'b10111110;  // 6
            4'h7: seg_decode = 8'b11100000;  // 7
            4'h8: seg_decode = 8'b11111110;  // 8
            4'h9: seg_decode = 8'b11110110;  // 9
            default: seg_decode = 8'b00000000;  // 空白
        endcase
    end
endfunction

// 扫描计数器
always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        scan_cnt <= 16'd0;
        scan_pos <= 3'd0;
    end else begin
        if (scan_cnt >= SCAN_FREQ - 1) begin
            scan_cnt <= 16'd0;
            scan_pos <= (scan_pos == 3'd5) ? 3'd0 : scan_pos + 1'b1;
        end else begin
            scan_cnt <= scan_cnt + 1'b1;
        end
    end
end

// BCD转换
always @(*) begin
    bcd_value = value;
    
    // 千位
    digit[5] = bcd_value / 1000;
    bcd_value = bcd_value % 1000;
    
    // 百位
    digit[4] = bcd_value / 100;
    bcd_value = bcd_value % 100;
    
    // 十位
    digit[3] = bcd_value / 10;
    
    // 个位
    digit[2] = bcd_value % 10;
    
    // 十分位（0.1秒）
    digit[1] = 4'd0;  // 预留
    digit[0] = 4'd0;  // 预留
end

// 重新组织数字显示
always @(*) begin
    if (precision_mode) begin
        // 精度模式：显示 XXX.X 格式
        case (scan_pos)
            3'd0: begin  // 最右位：十分位
                current_digit = (value % 10);
                dot_enable = 1'b0;
            end
            3'd1: begin  // 个位（带小数点）
                current_digit = (value / 10) % 10;
                dot_enable = 1'b1;
            end
            3'd2: begin  // 十位
                current_digit = (value / 100) % 10;
                dot_enable = 1'b0;
            end
            3'd3: begin  // 百位
                current_digit = (value / 1000) % 10;
                dot_enable = 1'b0;
            end
            3'd4: begin  // 千位
                current_digit = (value >= 10000) ? ((value / 10000) % 10) : 4'd0;
                dot_enable = 1'b0;
            end
            3'd5: begin  // 万位
                current_digit = (value >= 100000) ? ((value / 100000) % 10) : 4'd0;
                dot_enable = 1'b0;
            end
            default: begin
                current_digit = 4'd0;
                dot_enable = 1'b0;
            end
        endcase
    end else begin
        // 秒模式：显示整秒
        case (scan_pos)
            3'd0: begin  // 个位
                current_digit = ((value / 10) % 10);
                dot_enable = 1'b0;
            end
            3'd1: begin  // 十位
                current_digit = ((value / 100) % 10);
                dot_enable = 1'b0;
            end
            3'd2: begin  // 百位
                current_digit = ((value / 1000) % 10);
                dot_enable = 1'b0;
            end
            3'd3: begin  // 千位
                current_digit = (value >= 10000) ? ((value / 10000) % 10) : 4'd0;
                dot_enable = 1'b0;
            end
            3'd4: begin  // 万位
                current_digit = (value >= 100000) ? ((value / 100000) % 10) : 4'd0;
                dot_enable = 1'b0;
            end
            3'd5: begin  // 十万位
                current_digit = (value >= 1000000) ? ((value / 1000000) % 10) : 4'd0;
                dot_enable = 1'b0;
            end
            default: begin
                current_digit = 4'd0;
                dot_enable = 1'b0;
            end
        endcase
    end
end

// 输出控制
always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        seg <= 8'b00000000;
        dig <= 6'b111111;
    end else begin
        // 位选信号（低有效）
        dig <= ~(6'b000001 << scan_pos);
        
        // 段选信号
        if (current_digit == 4'd0 && scan_pos >= 3'd2 && 
            ((precision_mode && value < 1000) || (!precision_mode && value < 100))) begin
            // 前导零消隐
            seg <= 8'b00000000;
        end else begin
            seg <= seg_decode(current_digit);
            if (dot_enable) begin
                seg[0] <= 1'b1;  // 小数点
            end
        end
    end
end

endmodule