#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PI 3.14159265359
#define MAX_POINTS 360
#define DEG_TO_RAD(x) ((x) * PI / 180.0)
#define RAD_TO_DEG(x) ((x) * 180.0 / PI)

// 机构参数结构体（根据表格数据）
typedef struct {
    // 几何参数
    double l_OA;        // 机架长度 (mm)
    double l_AB;        // 连杆AB长度 (mm) 
    double l_BC;        // 连杆BC长度 (mm)
    double l_CD;        // 连杆CD长度 (mm)
    double l_DE;        // 连杆DE长度 (mm)
    double l_EF;        // 连杆EF长度 (mm)
    double alpha;       // 方向角 (度)
    double e;           // 偏心距 (mm)
    
    // 运动参数
    double n2;          // 曲柄转速 (r/min)
    double omega2;      // 曲柄角速度 (rad/s)
    
    // 力学参数
    double F_max;       // 最大工作阻力 (N)
    double m6;          // 滑块6质量 (kg)
    double J_flywheel;  // 飞轮转动惯量 (kg·m²)
} MechanismParams;

// 位置结构体
typedef struct {
    double x, y;
} Point2D;

// 运动学结果
typedef struct {
    double theta2;      // 曲柄转角 (rad)
    Point2D pos_A;      // A点位置
    Point2D pos_B;      // B点位置
    Point2D pos_C;      // C点位置
    Point2D pos_D;      // D点位置
    Point2D pos_E;      // E点位置
    Point2D pos_F;      // F点位置
    double s6;          // 滑块6位移 (mm)
    double v6;          // 滑块6速度 (mm/s)
    double a6;          // 滑块6加速度 (mm/s²)
    double theta3;      // 连杆3角度 (rad)
    double omega3;      // 连杆3角速度 (rad/s)
    double alpha3;      // 连杆3角加速度 (rad/s²)
} KinematicState;

// 动力学结果
typedef struct {
    double F_work;      // 工作阻力 (N)
    double F_inertia;   // 惯性力 (N)
    double M_drive;     // 驱动力矩 (N·m)
    double M_resist;    // 阻抗力矩 (N·m)
} DynamicForces;

// 初始化机构参数（使用表格第1组数据）
void init_mechanism_params(MechanismParams *params) {
    // 几何参数 (mm)
    params->l_OA = 65;      // loa
    params->l_AB = 100;     // 连杆长度
    params->l_BC = 16;      // 连杆长度
    params->l_CD = 100;     // 连杆长度
    params->l_DE = 40;      // 连杆长度
    params->l_EF = 20;      // 连杆长度
    params->alpha = 1.6;    // 方向角 (度)
    params->e = 1.1;        // 偏心距 (mm)
    
    // 运动参数
    params->n2 = 30;        // r/min
    params->omega2 = params->n2 * 2 * PI / 60;  // rad/s
    
    // 力学参数
    params->F_max = 6300;   // N
    params->m6 = 50;        // kg (假设值)
    params->J_flywheel = 0.5; // kg·m² (假设值)
}

// 求解四杆机构位置
int solve_four_bar_position(double l1, double l2, double l3, double l4, 
                           double theta2, double *theta3, double *theta4) {
    // 使用解析法求解四杆机构
    double A = 2 * l1 * l3 * cos(theta2) - 2 * l2 * l3;
    double B = 2 * l1 * l3 * sin(theta2);
    double C = l1*l1 + l2*l2 + l3*l3 - l4*l4 - 2*l1*l2*cos(theta2);
    
    double discriminant = B*B - 4*A*C;
    if (discriminant < 0) {
        return 0; // 无解
    }
    
    double tan_half_theta3_1 = (-B + sqrt(discriminant)) / (2*A);
    double tan_half_theta3_2 = (-B - sqrt(discriminant)) / (2*A);
    
    // 选择合适的解（通常选择第一个解）
    *theta3 = 2 * atan(tan_half_theta3_1);
    
    // 计算theta4
    double num = l1 * sin(theta2) + l3 * sin(*theta3);
    double den = l1 * cos(theta2) + l3 * cos(*theta3) - l2;
    *theta4 = atan2(num, den);
    
    return 1; // 有解
}

// 精确的运动学分析
void precise_kinematic_analysis(MechanismParams *params, KinematicState *state, double theta2) {
    state->theta2 = theta2;
    
    // A点位置（曲柄末端）
    state->pos_A.x = params->l_OA * cos(theta2);
    state->pos_A.y = params->l_OA * sin(theta2);
    
    // 求解连杆机构的角度
    double theta3, theta4;
    if (solve_four_bar_position(params->l_OA, params->l_AB, params->l_BC, params->l_CD,
                               theta2, &theta3, &theta4)) {
        state->theta3 = theta3;
        
        // B点位置
        state->pos_B.x = state->pos_A.x + params->l_AB * cos(theta3);
        state->pos_B.y = state->pos_A.y + params->l_AB * sin(theta3);
        
        // C点位置
        state->pos_C.x = state->pos_B.x + params->l_BC * cos(theta4);
        state->pos_C.y = state->pos_B.y + params->l_BC * sin(theta4);
        
        // 滑块6的位移（假设沿x轴运动）
        state->s6 = state->pos_C.x + params->e;
        
        // 速度分析（数值微分）
        double dt = 0.001; // 小时间步长
        double theta2_next = theta2 + params->omega2 * dt;
        
        KinematicState state_next;
        precise_kinematic_analysis(params, &state_next, theta2_next);
        
        state->v6 = (state_next.s6 - state->s6) / dt;
        state->omega3 = (state_next.theta3 - state->theta3) / dt;
        
        // 加速度分析
        double theta2_prev = theta2 - params->omega2 * dt;
        KinematicState state_prev;
        precise_kinematic_analysis(params, &state_prev, theta2_prev);
        
        state->a6 = (state_next.v6 - (state->s6 - state_prev.s6) / dt) / dt;
        state->alpha3 = (state->omega3 - (state->theta3 - state_prev.theta3) / dt) / dt;
    }
}

// 工作阻力函数（分段函数）
double calculate_work_resistance(double s, double s_max, double s_min, double F_max) {
    double stroke = s_max - s_min;
    double work_start = s_min + 0.05 * stroke;  // 工作行程开始
    double work_end = s_max - 0.05 * stroke;    // 工作行程结束
    
    if (s >= work_start && s <= work_end) {
        // 工作行程中，阻力为常数
        return F_max;
    } else {
        // 空行程，无工作阻力
        return 0.0;
    }
}

// 动力学分析
void dynamic_analysis(MechanismParams *params, KinematicState *state, 
                     DynamicForces *forces, double s_max, double s_min) {
    // 工作阻力
    forces->F_work = calculate_work_resistance(state->s6, s_max, s_min, params->F_max);
    
    // 惯性力
    forces->F_inertia = params->m6 * state->a6 / 1000.0; // 转换为N
    
    // 总阻力
    double F_total = forces->F_work + forces->F_inertia;
    
    // 阻抗力矩（简化计算）
    forces->M_resist = F_total * params->l_OA / 1000.0; // 转换为N·m
    
    // 驱动力矩（平衡条件）
    forces->M_drive = forces->M_resist;
}

// 生成机构运动简图数据
void generate_mechanism_diagram(MechanismParams *params, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("无法创建机构简图文件\n");
        return;
    }
    
    fprintf(fp, "# 六杆机构运动简图数据\n");
    fprintf(fp, "# 格式: 角度 A_x A_y B_x B_y C_x C_y\n");
    
    for (int i = 0; i < 12; i++) { // 每30度一个位置
        double theta2 = i * PI / 6;
        KinematicState state;
        precise_kinematic_analysis(params, &state, theta2);
        
        fprintf(fp, "%.1f %.3f %.3f %.3f %.3f %.3f %.3f\n",
                RAD_TO_DEG(theta2),
                state.pos_A.x, state.pos_A.y,
                state.pos_B.x, state.pos_B.y,
                state.pos_C.x, state.pos_C.y);
    }
    
    fclose(fp);
    printf("机构简图数据已保存到 %s\n", filename);
}

// 主分析程序
void comprehensive_analysis(MechanismParams *params) {
    printf("========================================\n");
    printf("      六杆机构综合分析程序\n");
    printf("========================================\n\n");
    
    // 输出机构参数
    printf("机构设计参数:\n");
    printf("  机架长度 l_OA = %.1f mm\n", params->l_OA);
    printf("  连杆长度 l_AB = %.1f mm\n", params->l_AB);
    printf("  连杆长度 l_BC = %.1f mm\n", params->l_BC);
    printf("  连杆长度 l_CD = %.1f mm\n", params->l_CD);
    printf("  方向角 α = %.1f°\n", params->alpha);
    printf("  偏心距 e = %.1f mm\n", params->e);
    printf("  曲柄转速 n₂ = %.1f r/min\n", params->n2);
    printf("  角速度 ω₂ = %.3f rad/s\n", params->omega2);
    printf("  最大工作阻力 = %.0f N\n", params->F_max);
    printf("\n");
    
    // 分配内存
    KinematicState *states = malloc(MAX_POINTS * sizeof(KinematicState));
    DynamicForces *forces = malloc(MAX_POINTS * sizeof(DynamicForces));
    
    if (!states || !forces) {
        printf("内存分配失败\n");
        return;
    }
    
    // 运动学分析
    printf("进行运动学分析...\n");
    double s_max = -1000000, s_min = 1000000;
    double v_max = -1000000, v_min = 1000000;
    double a_max = -1000000, a_min = 1000000;
    
    for (int i = 0; i < MAX_POINTS; i++) {
        double theta2 = 2 * PI * i / MAX_POINTS;
        precise_kinematic_analysis(params, &states[i], theta2);
        
        // 统计极值
        if (states[i].s6 > s_max) s_max = states[i].s6;
        if (states[i].s6 < s_min) s_min = states[i].s6;
        if (states[i].v6 > v_max) v_max = states[i].v6;
        if (states[i].v6 < v_min) v_min = states[i].v6;
        if (states[i].a6 > a_max) a_max = states[i].a6;
        if (states[i].a6 < a_min) a_min = states[i].a6;
    }
    
    printf("运动学分析结果:\n");
    printf("  滑块6行程: %.2f mm\n", s_max - s_min);
    printf("  位移范围: %.2f ~ %.2f mm\n", s_min, s_max);
    printf("  速度范围: %.2f ~ %.2f mm/s\n", v_min, v_max);
    printf("  加速度范围: %.2f ~ %.2f mm/s²\n", a_min, a_max);
    printf("\n");
    
    // 动力学分析
    printf("进行动力学分析...\n");
    for (int i = 0; i < MAX_POINTS; i++) {
        dynamic_analysis(params, &states[i], &forces[i], s_max, s_min);
    }
    
    // 输出详细结果文件
    FILE *fp_detail = fopen("detailed_analysis.txt", "w");
    if (fp_detail != NULL) {
        fprintf(fp_detail, "角度(°)\t位移(mm)\t速度(mm/s)\t加速度(mm/s²)\t工作阻力(N)\t惯性力(N)\t驱动力矩(N·m)\n");
        for (int i = 0; i < MAX_POINTS; i++) {
            fprintf(fp_detail, "%.1f\t%.3f\t%.3f\t%.3f\t%.1f\t%.3f\t%.6f\n",
                    RAD_TO_DEG(states[i].theta2),
                    states[i].s6,
                    states[i].v6,
                    states[i].a6,
                    forces[i].F_work,
                    forces[i].F_inertia,
                    forces[i].M_drive);
        }
        fclose(fp_detail);
        printf("详细分析结果已保存到 detailed_analysis.txt\n");
    }
    
    // 生成机构简图数据
    generate_mechanism_diagram(params, "mechanism_diagram.txt");
    
    // 输出关键位置的结果
    printf("\n关键位置分析结果:\n");
    printf("角度(°)\t位移(mm)\t速度(mm/s)\t加速度(mm/s²)\t工作阻力(N)\n");
    printf("----------------------------------------------------------------\n");
    for (int i = 0; i < MAX_POINTS; i += 45) {
        printf("%.0f\t%.3f\t\t%.3f\t\t%.3f\t\t%.0f\n",
               RAD_TO_DEG(states[i].theta2),
               states[i].s6,
               states[i].v6,
               states[i].a6,
               forces[i].F_work);
    }
    
    // 释放内存
    free(states);
    free(forces);
    
    printf("\n========================================\n");
    printf("分析完成！生成的文件:\n");
    printf("  - detailed_analysis.txt (详细分析结果)\n");
    printf("  - mechanism_diagram.txt (机构简图数据)\n");
    printf("========================================\n");
}

int main() {
    MechanismParams params;
    
    // 初始化参数
    init_mechanism_params(&params);
    
    // 执行综合分析
    comprehensive_analysis(&params);
    
    return 0;
}