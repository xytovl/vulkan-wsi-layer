
/*
 * Copyright (c) 2019 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file
 * @brief Implements factory methods for obtaining the specific surface and swapchain implementations.
 */

#include "wsi_factory.hpp"
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <new>
#include <vulkan/vk_icd.h>

#include "headless/surface_properties.hpp"
#include "headless/swapchain.hpp"

namespace wsi
{

surface_properties *get_surface_properties(VkSurfaceKHR surface)
{
   VkIcdSurfaceBase *surface_base = reinterpret_cast<VkIcdSurfaceBase *>(surface);

   switch (surface_base->platform)
   {
   case VK_ICD_WSI_PLATFORM_HEADLESS:
      return &headless::surface_properties::get_instance();
   default:
      return nullptr;
   }
}

template <typename swapchain_type>
static swapchain_base *allocate_swapchain(layer::device_private_data &dev_data, const VkAllocationCallbacks *pAllocator)
{
   if (!pAllocator)
   {
      return new swapchain_type(dev_data, pAllocator);
   }
   void *memory = pAllocator->pfnAllocation(pAllocator->pUserData, sizeof(swapchain_type), alignof(swapchain_type),
                                            VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   return new (memory) swapchain_type(dev_data, pAllocator);
}

swapchain_base *allocate_surface_swapchain(VkSurfaceKHR surface, layer::device_private_data &dev_data,
                                           const VkAllocationCallbacks *pAllocator)
{
   VkIcdSurfaceBase *surface_base = reinterpret_cast<VkIcdSurfaceBase *>(surface);

   switch (surface_base->platform)
   {
   case VK_ICD_WSI_PLATFORM_HEADLESS:
      return allocate_swapchain<wsi::headless::swapchain>(dev_data, pAllocator);
   default:
      return nullptr;
   }
}

void destroy_surface_swapchain(swapchain_base *swapchain, const VkAllocationCallbacks *pAllocator)
{
   assert(swapchain);

   if (!pAllocator)
   {
      delete swapchain;
   }
   else
   {
      swapchain->~swapchain_base();
      pAllocator->pfnFree(pAllocator->pUserData, reinterpret_cast<void *>(swapchain));
   }
}

} // namespace wsi