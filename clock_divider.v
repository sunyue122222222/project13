/*
 * 时钟分频模块
 * 从50MHz系统时钟生成1Hz、2Hz、4Hz时钟信号
 */

module clock_divider(
    input wire clk,        // 50MHz输入时钟
    input wire rst_n,      // 复位信号
    output reg clk_1hz,    // 1Hz输出时钟
    output reg clk_2hz,    // 2Hz输出时钟
    output reg clk_4hz     // 4Hz输出时钟
);

// 计数器位宽计算：
// 1Hz: 50MHz / 1Hz = 50,000,000 -> 需要26位计数器
// 2Hz: 50MHz / 2Hz = 25,000,000 -> 需要25位计数器  
// 4Hz: 50MHz / 4Hz = 12,500,000 -> 需要24位计数器

parameter CNT_1HZ = 26'd25_000_000;  // 1Hz分频计数值（0.1秒精度）
parameter CNT_2HZ = 25'd12_500_000;  // 2Hz分频计数值
parameter CNT_4HZ = 24'd6_250_000;   // 4Hz分频计数值

reg [25:0] cnt_1hz;
reg [24:0] cnt_2hz;
reg [23:0] cnt_4hz;

// 1Hz时钟生成（实际为10Hz，用于0.1秒精度）
always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        cnt_1hz <= 26'd0;
        clk_1hz <= 1'b0;
    end else begin
        if (cnt_1hz >= CNT_1HZ - 1) begin
            cnt_1hz <= 26'd0;
            clk_1hz <= ~clk_1hz;
        end else begin
            cnt_1hz <= cnt_1hz + 1'b1;
        end
    end
end

// 2Hz时钟生成（实际为20Hz）
always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        cnt_2hz <= 25'd0;
        clk_2hz <= 1'b0;
    end else begin
        if (cnt_2hz >= CNT_2HZ - 1) begin
            cnt_2hz <= 25'd0;
            clk_2hz <= ~clk_2hz;
        end else begin
            cnt_2hz <= cnt_2hz + 1'b1;
        end
    end
end

// 4Hz时钟生成（实际为40Hz）
always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        cnt_4hz <= 24'd0;
        clk_4hz <= 1'b0;
    end else begin
        if (cnt_4hz >= CNT_4HZ - 1) begin
            cnt_4hz <= 24'd0;
            clk_4hz <= ~clk_4hz;
        end else begin
            cnt_4hz <= cnt_4hz + 1'b1;
        end
    end
end

endmodule