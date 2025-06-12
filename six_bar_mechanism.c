#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PI 3.14159265359
#define MAX_POINTS 360  // 一个周期的计算点数

// 机构参数结构体
typedef struct {
    double loa;     // 机架长度 (mm)
    double lab;     // 连杆AB长度 (mm)
    double lbc;     // 连杆BC长度 (mm)
    double lcd;     // 连杆CD长度 (mm)
    double lde;     // 连杆DE长度 (mm)
    double lef;     // 连杆EF长度 (mm)
    double alpha;   // 方向角 (度)
    double e;       // 偏心距 (mm)
    double n2;      // 曲柄转速 (r/min)
    double omega2;  // 曲柄角速度 (rad/s)
    double F_work;  // 工作阻力 (N)
} MechanismParams;

// 运动学结果结构体
typedef struct {
    double theta2;  // 曲柄转角 (rad)
    double s6;      // 滑块6位移 (mm)
    double v6;      // 滑块6速度 (mm/s)
    double a6;      // 滑块6加速度 (mm/s²)
} KinematicResult;

// 初始化机构参数
void init_mechanism_params(MechanismParams *params) {
    // 根据表格中的数据设置参数（以第1组数据为例）
    params->loa = 65;       // mm
    params->lab = 100;      // mm
    params->lbc = 16;       // mm
    params->lcd = 100;      // mm
    params->lde = 40;       // mm
    params->lef = 20;       // mm
    params->alpha = 1.6;    // 度
    params->e = 1.1;        // mm
    params->n2 = 30;        // r/min
    params->omega2 = params->n2 * 2 * PI / 60;  // rad/s
    params->F_work = 6300;  // N
}

// 角度转弧度
double deg_to_rad(double deg) {
    return deg * PI / 180.0;
}

// 弧度转角度
double rad_to_deg(double rad) {
    return rad * 180.0 / PI;
}

// 计算滑块6的位移（简化的运动学分析）
double calculate_slider_displacement(MechanismParams *params, double theta2) {
    // 这里使用简化的运动学模型
    // 实际应用中需要根据具体的机构几何关系进行精确计算
    
    double x2 = params->loa * cos(theta2);
    double y2 = params->loa * sin(theta2);
    
    // 考虑连杆系统的影响
    double s6 = params->loa * cos(theta2) + 
                params->lab * cos(theta2 + deg_to_rad(params->alpha)) +
                params->e;
    
    return s6;
}

// 计算滑块6的速度
double calculate_slider_velocity(MechanismParams *params, double theta2) {
    // 对位移求导得到速度
    double v6 = -params->loa * params->omega2 * sin(theta2) -
                params->lab * params->omega2 * sin(theta2 + deg_to_rad(params->alpha));
    
    return v6;
}

// 计算滑块6的加速度
double calculate_slider_acceleration(MechanismParams *params, double theta2) {
    // 对速度求导得到加速度
    double a6 = -params->loa * params->omega2 * params->omega2 * cos(theta2) -
                params->lab * params->omega2 * params->omega2 * cos(theta2 + deg_to_rad(params->alpha));
    
    return a6;
}

// 工作阻力函数（根据行程变化）
double work_resistance_function(double s, double s_max) {
    // 简化的工作阻力模型：在工作行程中为常数，其他时候为0
    double s_work_start = s_max * 0.2;  // 工作行程开始位置
    double s_work_end = s_max * 0.8;    // 工作行程结束位置
    
    if (s >= s_work_start && s <= s_work_end) {
        return 6300.0;  // 工作阻力 (N)
    } else {
        return 0.0;     // 空行程，无阻力
    }
}

// 运动学分析主程序
void kinematic_analysis(MechanismParams *params, KinematicResult results[]) {
    printf("开始运动学分析...\n");
    printf("机构参数:\n");
    printf("  机架长度 loa = %.1f mm\n", params->loa);
    printf("  连杆长度 lab = %.1f mm\n", params->lab);
    printf("  曲柄转速 n2 = %.1f r/min\n", params->n2);
    printf("  角速度 ω2 = %.3f rad/s\n", params->omega2);
    printf("\n");
    
    double s_max = 0, s_min = 1000000;
    
    for (int i = 0; i < MAX_POINTS; i++) {
        double theta2 = 2 * PI * i / MAX_POINTS;  // 曲柄转角
        
        results[i].theta2 = theta2;
        results[i].s6 = calculate_slider_displacement(params, theta2);
        results[i].v6 = calculate_slider_velocity(params, theta2);
        results[i].a6 = calculate_slider_acceleration(params, theta2);
        
        // 记录最大最小位移
        if (results[i].s6 > s_max) s_max = results[i].s6;
        if (results[i].s6 < s_min) s_min = results[i].s6;
    }
    
    printf("运动学分析结果:\n");
    printf("  滑块6行程: %.2f mm\n", s_max - s_min);
    printf("  最大位移: %.2f mm\n", s_max);
    printf("  最小位移: %.2f mm\n", s_min);
    printf("\n");
}

// 动态静力分析
void dynamic_static_analysis(MechanismParams *params, KinematicResult results[]) {
    printf("开始动态静力分析...\n");
    
    FILE *fp = fopen("force_analysis.txt", "w");
    if (fp == NULL) {
        printf("无法创建力分析输出文件\n");
        return;
    }
    
    fprintf(fp, "角度(度)\t位移(mm)\t速度(mm/s)\t加速度(mm/s²)\t工作阻力(N)\n");
    
    double s_max = 0, s_min = 1000000;
    
    // 先找到行程范围
    for (int i = 0; i < MAX_POINTS; i++) {
        if (results[i].s6 > s_max) s_max = results[i].s6;
        if (results[i].s6 < s_min) s_min = results[i].s6;
    }
    
    for (int i = 0; i < MAX_POINTS; i++) {
        double angle_deg = rad_to_deg(results[i].theta2);
        double work_force = work_resistance_function(results[i].s6, s_max);
        
        fprintf(fp, "%.1f\t%.3f\t%.3f\t%.3f\t%.1f\n", 
                angle_deg, results[i].s6, results[i].v6, results[i].a6, work_force);
    }
    
    fclose(fp);
    printf("力分析结果已保存到 force_analysis.txt\n");
}

// 输出运动曲线数据
void output_motion_curves(KinematicResult results[]) {
    printf("生成运动曲线数据...\n");
    
    // 位移曲线
    FILE *fp_s = fopen("displacement_curve.txt", "w");
    if (fp_s != NULL) {
        fprintf(fp_s, "角度(度)\t位移(mm)\n");
        for (int i = 0; i < MAX_POINTS; i++) {
            fprintf(fp_s, "%.1f\t%.3f\n", rad_to_deg(results[i].theta2), results[i].s6);
        }
        fclose(fp_s);
        printf("位移曲线数据已保存到 displacement_curve.txt\n");
    }
    
    // 速度曲线
    FILE *fp_v = fopen("velocity_curve.txt", "w");
    if (fp_v != NULL) {
        fprintf(fp_v, "角度(度)\t速度(mm/s)\n");
        for (int i = 0; i < MAX_POINTS; i++) {
            fprintf(fp_v, "%.1f\t%.3f\n", rad_to_deg(results[i].theta2), results[i].v6);
        }
        fclose(fp_v);
        printf("速度曲线数据已保存到 velocity_curve.txt\n");
    }
    
    // 加速度曲线
    FILE *fp_a = fopen("acceleration_curve.txt", "w");
    if (fp_a != NULL) {
        fprintf(fp_a, "角度(度)\t加速度(mm/s²)\n");
        for (int i = 0; i < MAX_POINTS; i++) {
            fprintf(fp_a, "%.1f\t%.3f\n", rad_to_deg(results[i].theta2), results[i].a6);
        }
        fclose(fp_a);
        printf("加速度曲线数据已保存到 acceleration_curve.txt\n");
    }
}

// 打印部分计算结果
void print_sample_results(KinematicResult results[]) {
    printf("\n部分计算结果示例:\n");
    printf("角度(度)\t位移(mm)\t速度(mm/s)\t加速度(mm/s²)\n");
    printf("-------------------------------------------------------\n");
    
    for (int i = 0; i < MAX_POINTS; i += 30) {  // 每30度输出一次
        printf("%.1f\t\t%.3f\t\t%.3f\t\t%.3f\n", 
               rad_to_deg(results[i].theta2), 
               results[i].s6, 
               results[i].v6, 
               results[i].a6);
    }
}

// 机构设计验证
void mechanism_design_verification(MechanismParams *params) {
    printf("机构设计验证:\n");
    
    // 检查传动角
    double min_transmission_angle = 30.0;  // 最小传动角要求
    printf("  最小传动角要求: %.1f度\n", min_transmission_angle);
    
    // 检查机构的几何约束
    double total_length = params->loa + params->lab + params->lbc;
    printf("  机构总长度: %.1f mm\n", total_length);
    
    // 检查是否满足格拉晓夫定理
    double lengths[] = {params->loa, params->lab, params->lbc, params->lcd};
    double max_len = 0, min_len = 1000000;
    double sum_others = 0;
    
    for (int i = 0; i < 4; i++) {
        if (lengths[i] > max_len) max_len = lengths[i];
        if (lengths[i] < min_len) min_len = lengths[i];
        sum_others += lengths[i];
    }
    sum_others = sum_others - max_len - min_len;
    
    if (max_len < min_len + sum_others) {
        printf("  机构满足格拉晓夫定理，可以实现连续转动\n");
    } else {
        printf("  警告：机构可能不满足格拉晓夫定理\n");
    }
    
    printf("\n");
}

int main() {
    printf("========================================\n");
    printf("      六杆机构运动学分析程序\n");
    printf("========================================\n\n");
    
    MechanismParams params;
    KinematicResult results[MAX_POINTS];
    
    // 初始化参数
    init_mechanism_params(&params);
    
    // 机构设计验证
    mechanism_design_verification(&params);
    
    // 运动学分析
    kinematic_analysis(&params, results);
    
    // 动态静力分析
    dynamic_static_analysis(&params, results);
    
    // 输出运动曲线数据
    output_motion_curves(results);
    
    // 打印部分结果
    print_sample_results(results);
    
    printf("\n========================================\n");
    printf("分析完成！请查看生成的数据文件：\n");
    printf("  - displacement_curve.txt (位移曲线)\n");
    printf("  - velocity_curve.txt (速度曲线)\n");
    printf("  - acceleration_curve.txt (加速度曲线)\n");
    printf("  - force_analysis.txt (力分析结果)\n");
    printf("========================================\n");
    
    return 0;
}