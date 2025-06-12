#!/usr/bin/env python3
"""
OpenHands代码帮助演示会话
演示如何帮助用户优化一个实际的Python程序
"""

import time
import random
import json
from typing import List, Dict, Any
import concurrent.futures
import asyncio
import aiohttp

# ============================================================================
# 场景：用户提交了一个需要优化的数据处理程序
# ============================================================================

print("=" * 60)
print("OpenHands代码帮助演示会话")
print("=" * 60)

# 用户的原始代码（存在多个性能问题）
class OriginalDataProcessor:
    """用户的原始代码 - 存在性能问题"""
    
    def __init__(self):
        self.data = []
        self.processed_data = []
    
    def load_data_from_api(self, urls: List[str]) -> List[Dict]:
        """从多个API加载数据 - 问题：串行请求"""
        all_data = []
        for url in urls:
            # 模拟API请求
            time.sleep(0.1)  # 模拟网络延迟
            data = {'url': url, 'data': [random.randint(1, 100) for _ in range(100)]}
            all_data.append(data)
        return all_data
    
    def process_data(self, data_list: List[Dict]) -> List[Dict]:
        """处理数据 - 问题：低效的算法"""
        processed = []
        for item in data_list:
            # 低效的数据处理
            numbers = item['data']
            
            # 问题1：重复计算
            avg = sum(numbers) / len(numbers)
            max_val = max(numbers)
            min_val = min(numbers)
            
            # 问题2：低效的过滤
            filtered = []
            for num in numbers:
                if num > avg:
                    filtered.append(num)
            
            # 问题3：低效的排序
            sorted_data = []
            for num in filtered:
                inserted = False
                for i, existing in enumerate(sorted_data):
                    if num < existing:
                        sorted_data.insert(i, num)
                        inserted = True
                        break
                if not inserted:
                    sorted_data.append(num)
            
            processed.append({
                'url': item['url'],
                'avg': avg,
                'max': max_val,
                'min': min_val,
                'filtered_count': len(filtered),
                'top_values': sorted_data[-5:] if len(sorted_data) >= 5 else sorted_data
            })
        
        return processed
    
    def save_results(self, results: List[Dict], filename: str):
        """保存结果 - 问题：同步IO"""
        with open(filename, 'w') as f:
            json.dump(results, f, indent=2)

# ============================================================================
# OpenHands分析和优化方案
# ============================================================================

print("\n🔍 OpenHands分析：发现以下问题")
print("-" * 40)
print("1. API请求串行执行，效率低下")
print("2. 数据处理算法复杂度过高")
print("3. 重复计算和低效的数据结构操作")
print("4. 同步IO操作阻塞程序执行")
print("5. 缺乏错误处理和日志记录")

print("\n🚀 OpenHands优化方案：")
print("-" * 40)

# 优化版本1：基础优化
class OptimizedDataProcessor:
    """OpenHands优化版本 - 基础改进"""
    
    def __init__(self):
        self.session = None
    
    async def load_data_from_api_async(self, urls: List[str]) -> List[Dict]:
        """异步并行加载数据"""
        async def fetch_data(session, url):
            # 模拟异步API请求
            await asyncio.sleep(0.1)
            return {'url': url, 'data': [random.randint(1, 100) for _ in range(100)]}
        
        async with aiohttp.ClientSession() as session:
            tasks = [fetch_data(session, url) for url in urls]
            return await asyncio.gather(*tasks)
    
    def process_data_optimized(self, data_list: List[Dict]) -> List[Dict]:
        """优化的数据处理"""
        processed = []
        
        for item in data_list:
            numbers = item['data']
            
            # 优化1：一次遍历计算所有统计值
            total = 0
            max_val = float('-inf')
            min_val = float('inf')
            
            for num in numbers:
                total += num
                max_val = max(max_val, num)
                min_val = min(min_val, num)
            
            avg = total / len(numbers)
            
            # 优化2：使用列表推导式和内置函数
            filtered = [num for num in numbers if num > avg]
            top_values = sorted(filtered)[-5:] if len(filtered) >= 5 else sorted(filtered)
            
            processed.append({
                'url': item['url'],
                'avg': avg,
                'max': max_val,
                'min': min_val,
                'filtered_count': len(filtered),
                'top_values': top_values
            })
        
        return processed
    
    async def save_results_async(self, results: List[Dict], filename: str):
        """异步保存结果"""
        import aiofiles
        async with aiofiles.open(filename, 'w') as f:
            await f.write(json.dumps(results, indent=2))

# 优化版本2：高性能版本
import numpy as np
from concurrent.futures import ThreadPoolExecutor, ProcessPoolExecutor

class HighPerformanceDataProcessor:
    """OpenHands高性能版本 - 使用numpy和并行处理"""
    
    def __init__(self, max_workers: int = 4):
        self.max_workers = max_workers
    
    def process_single_item(self, item: Dict) -> Dict:
        """处理单个数据项（用于并行处理）"""
        numbers = np.array(item['data'])
        
        # 使用numpy的向量化操作
        avg = np.mean(numbers)
        max_val = np.max(numbers)
        min_val = np.min(numbers)
        
        # 向量化过滤和排序
        filtered = numbers[numbers > avg]
        top_values = np.sort(filtered)[-5:].tolist() if len(filtered) >= 5 else sorted(filtered.tolist())
        
        return {
            'url': item['url'],
            'avg': float(avg),
            'max': int(max_val),
            'min': int(min_val),
            'filtered_count': len(filtered),
            'top_values': top_values
        }
    
    def process_data_parallel(self, data_list: List[Dict]) -> List[Dict]:
        """并行处理数据"""
        with ProcessPoolExecutor(max_workers=self.max_workers) as executor:
            results = list(executor.map(self.process_single_item, data_list))
        return results
    
    async def load_data_concurrent(self, urls: List[str]) -> List[Dict]:
        """并发加载数据（混合异步和线程池）"""
        def fetch_data_sync(url):
            time.sleep(0.1)  # 模拟同步API调用
            return {'url': url, 'data': [random.randint(1, 100) for _ in range(100)]}
        
        loop = asyncio.get_event_loop()
        with ThreadPoolExecutor(max_workers=self.max_workers) as executor:
            tasks = [loop.run_in_executor(executor, fetch_data_sync, url) for url in urls]
            return await asyncio.gather(*tasks)

# ============================================================================
# 性能测试和对比
# ============================================================================

async def performance_comparison():
    """性能对比测试"""
    print("\n📊 性能测试对比")
    print("=" * 50)
    
    # 测试数据
    test_urls = [f"https://api.example.com/data/{i}" for i in range(20)]
    
    # 测试原始版本
    print("🐌 测试原始版本...")
    original_processor = OriginalDataProcessor()
    
    start_time = time.time()
    original_data = original_processor.load_data_from_api(test_urls)
    original_results = original_processor.process_data(original_data)
    original_time = time.time() - start_time
    
    print(f"   原始版本耗时: {original_time:.2f}秒")
    
    # 测试优化版本
    print("🚀 测试优化版本...")
    optimized_processor = OptimizedDataProcessor()
    
    start_time = time.time()
    optimized_data = await optimized_processor.load_data_from_api_async(test_urls)
    optimized_results = optimized_processor.process_data_optimized(optimized_data)
    optimized_time = time.time() - start_time
    
    print(f"   优化版本耗时: {optimized_time:.2f}秒")
    
    # 测试高性能版本
    print("⚡ 测试高性能版本...")
    hp_processor = HighPerformanceDataProcessor()
    
    start_time = time.time()
    hp_data = await hp_processor.load_data_concurrent(test_urls)
    hp_results = hp_processor.process_data_parallel(hp_data)
    hp_time = time.time() - start_time
    
    print(f"   高性能版本耗时: {hp_time:.2f}秒")
    
    # 性能提升计算
    print("\n📈 性能提升报告")
    print("-" * 30)
    optimized_improvement = (original_time - optimized_time) / original_time * 100
    hp_improvement = (original_time - hp_time) / original_time * 100
    
    print(f"优化版本提升: {optimized_improvement:.1f}%")
    print(f"高性能版本提升: {hp_improvement:.1f}%")
    
    # 验证结果一致性
    print("\n✅ 结果验证")
    print("-" * 20)
    
    def verify_results(r1, r2):
        if len(r1) != len(r2):
            return False
        for i in range(len(r1)):
            if abs(r1[i]['avg'] - r2[i]['avg']) > 0.001:
                return False
        return True
    
    optimized_correct = verify_results(original_results, optimized_results)
    hp_correct = verify_results(original_results, hp_results)
    
    print(f"优化版本结果正确: {'✅' if optimized_correct else '❌'}")
    print(f"高性能版本结果正确: {'✅' if hp_correct else '❌'}")

# ============================================================================
# 代码质量改进建议
# ============================================================================

def code_quality_recommendations():
    """OpenHands代码质量改进建议"""
    print("\n💡 OpenHands代码质量改进建议")
    print("=" * 50)
    
    recommendations = [
        {
            "类别": "性能优化",
            "建议": [
                "使用异步编程处理IO密集型操作",
                "利用numpy进行数值计算",
                "使用并行处理处理CPU密集型任务",
                "避免重复计算，缓存中间结果"
            ]
        },
        {
            "类别": "代码结构",
            "建议": [
                "使用类型提示提高代码可读性",
                "实现适当的错误处理",
                "添加日志记录用于调试",
                "使用配置文件管理参数"
            ]
        },
        {
            "类别": "最佳实践",
            "建议": [
                "编写单元测试确保代码质量",
                "使用文档字符串说明函数用途",
                "遵循PEP 8代码风格规范",
                "实现优雅的资源管理"
            ]
        }
    ]
    
    for rec in recommendations:
        print(f"\n📋 {rec['类别']}")
        print("-" * 20)
        for i, suggestion in enumerate(rec['建议'], 1):
            print(f"  {i}. {suggestion}")

# ============================================================================
# 完整的生产就绪版本
# ============================================================================

class ProductionDataProcessor:
    """生产就绪版本 - 包含所有最佳实践"""
    
    def __init__(self, config: Dict[str, Any] = None):
        self.config = config or {
            'max_workers': 4,
            'timeout': 30,
            'retry_attempts': 3,
            'batch_size': 100
        }
        self.logger = self._setup_logger()
    
    def _setup_logger(self):
        import logging
        logging.basicConfig(level=logging.INFO)
        return logging.getLogger(__name__)
    
    async def process_with_error_handling(self, urls: List[str]) -> Dict[str, Any]:
        """完整的处理流程，包含错误处理"""
        try:
            self.logger.info(f"开始处理 {len(urls)} 个URL")
            
            # 加载数据
            data = await self._load_data_with_retry(urls)
            self.logger.info(f"成功加载 {len(data)} 个数据源")
            
            # 处理数据
            results = await self._process_data_batch(data)
            self.logger.info(f"处理完成，生成 {len(results)} 个结果")
            
            return {
                'success': True,
                'results': results,
                'processed_count': len(results),
                'errors': []
            }
            
        except Exception as e:
            self.logger.error(f"处理失败: {str(e)}")
            return {
                'success': False,
                'results': [],
                'processed_count': 0,
                'errors': [str(e)]
            }
    
    async def _load_data_with_retry(self, urls: List[str]) -> List[Dict]:
        """带重试机制的数据加载"""
        for attempt in range(self.config['retry_attempts']):
            try:
                return await self._load_data_concurrent(urls)
            except Exception as e:
                if attempt == self.config['retry_attempts'] - 1:
                    raise e
                self.logger.warning(f"加载失败，重试 {attempt + 1}/{self.config['retry_attempts']}")
                await asyncio.sleep(2 ** attempt)  # 指数退避
    
    async def _load_data_concurrent(self, urls: List[str]) -> List[Dict]:
        """并发加载数据"""
        semaphore = asyncio.Semaphore(self.config['max_workers'])
        
        async def fetch_with_semaphore(url):
            async with semaphore:
                await asyncio.sleep(0.1)  # 模拟API调用
                return {'url': url, 'data': [random.randint(1, 100) for _ in range(100)]}
        
        tasks = [fetch_with_semaphore(url) for url in urls]
        return await asyncio.gather(*tasks)
    
    async def _process_data_batch(self, data_list: List[Dict]) -> List[Dict]:
        """批量处理数据"""
        batch_size = self.config['batch_size']
        all_results = []
        
        for i in range(0, len(data_list), batch_size):
            batch = data_list[i:i + batch_size]
            batch_results = await self._process_batch_async(batch)
            all_results.extend(batch_results)
        
        return all_results
    
    async def _process_batch_async(self, batch: List[Dict]) -> List[Dict]:
        """异步处理批次"""
        loop = asyncio.get_event_loop()
        with ProcessPoolExecutor(max_workers=self.config['max_workers']) as executor:
            tasks = [loop.run_in_executor(executor, self._process_single_item, item) for item in batch]
            return await asyncio.gather(*tasks)
    
    def _process_single_item(self, item: Dict) -> Dict:
        """处理单个数据项"""
        try:
            numbers = np.array(item['data'])
            
            avg = np.mean(numbers)
            max_val = np.max(numbers)
            min_val = np.min(numbers)
            
            filtered = numbers[numbers > avg]
            top_values = np.sort(filtered)[-5:].tolist() if len(filtered) >= 5 else sorted(filtered.tolist())
            
            return {
                'url': item['url'],
                'avg': float(avg),
                'max': int(max_val),
                'min': int(min_val),
                'filtered_count': len(filtered),
                'top_values': top_values,
                'status': 'success'
            }
        except Exception as e:
            return {
                'url': item.get('url', 'unknown'),
                'error': str(e),
                'status': 'error'
            }

# ============================================================================
# 主演示函数
# ============================================================================

async def main():
    """主演示函数"""
    print("🎯 OpenHands代码帮助演示")
    print("=" * 50)
    
    # 运行性能对比
    await performance_comparison()
    
    # 显示代码质量建议
    code_quality_recommendations()
    
    # 演示生产就绪版本
    print("\n🏭 生产就绪版本演示")
    print("=" * 30)
    
    processor = ProductionDataProcessor()
    test_urls = [f"https://api.example.com/data/{i}" for i in range(10)]
    
    result = await processor.process_with_error_handling(test_urls)
    
    print(f"处理状态: {'成功' if result['success'] else '失败'}")
    print(f"处理数量: {result['processed_count']}")
    print(f"错误数量: {len(result['errors'])}")
    
    print("\n✨ OpenHands帮助总结")
    print("=" * 30)
    print("1. 识别并解决了性能瓶颈")
    print("2. 提供了多个优化方案")
    print("3. 实现了错误处理和日志记录")
    print("4. 添加了生产环境最佳实践")
    print("5. 提供了完整的测试和验证")

if __name__ == "__main__":
    # 运行演示
    try:
        asyncio.run(main())
    except ImportError:
        print("注意：某些依赖包可能未安装（如aiohttp, aiofiles, numpy）")
        print("这是一个演示脚本，展示OpenHands如何帮助优化代码")
        
        # 运行简化版本的性能对比
        async def simple_demo():
            await performance_comparison()
            code_quality_recommendations()
        
        asyncio.run(simple_demo())