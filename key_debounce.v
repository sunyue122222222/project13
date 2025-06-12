/*
 * 按键消抖模块
 * 消除按键抖动并生成单脉冲信号
 */

module key_debounce(
    input wire clk,         // 系统时钟
    input wire rst_n,       // 复位信号
    input wire key_in,      // 按键输入（低有效）
    output reg key_out,     // 消抖后的按键状态
    output reg key_pulse    // 按键按下脉冲（单脉冲）
);

parameter DEBOUNCE_TIME = 20'd1_000_000;  // 消抖时间：20ms @ 50MHz

reg [19:0] cnt;
reg key_sync_0, key_sync_1;
reg key_stable;
reg key_stable_d1;

// 同步器，防止亚稳态
always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        key_sync_0 <= 1'b1;  // 按键默认高电平
        key_sync_1 <= 1'b1;
    end else begin
        key_sync_0 <= key_in;
        key_sync_1 <= key_sync_0;
    end
end

// 消抖计数器
always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        cnt <= 20'd0;
        key_stable <= 1'b1;
    end else begin
        if (key_sync_1 != key_stable) begin
            cnt <= cnt + 1'b1;
            if (cnt >= DEBOUNCE_TIME - 1) begin
                cnt <= 20'd0;
                key_stable <= key_sync_1;
            end
        end else begin
            cnt <= 20'd0;
        end
    end
end

// 生成输出信号
always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        key_out <= 1'b1;
        key_stable_d1 <= 1'b1;
        key_pulse <= 1'b0;
    end else begin
        key_out <= key_stable;
        key_stable_d1 <= key_stable;
        // 检测下降沿（按键按下）
        key_pulse <= key_stable_d1 & (~key_stable);
    end
end

endmodule