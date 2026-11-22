/**
 * ============================================================================
 * ShopSphere Admin Portal Script
 * Interacts with Drogon C++ Admin Endpoints & Machine Learning Engines
 * ============================================================================
 */

// Global Admin Toast
function showAdminToast(message, type = 'success') {
  let container = document.getElementById('admin-toast-container');
  if (!container) {
    container = document.createElement('div');
    container.id = 'admin-toast-container';
    container.style.cssText = 'position: fixed; bottom: 20px; right: 20px; z-index: 10000; display: flex; flex-direction: column; gap: 8px;';
    document.body.appendChild(container);
  }

  const toast = document.createElement('div');
  const bg = type === 'success' ? '#10b981' : (type === 'error' ? '#ef4444' : '#3b82f6');
  toast.style.cssText = `background: #0f172a; color: #fff; border-left: 4px solid ${bg}; padding: 12px 18px; border-radius: 6px; font-size: 0.88rem; box-shadow: 0 10px 15px -3px rgba(0,0,0,0.3); display: flex; align-items: center; gap: 10px; min-width: 280px;`;
  toast.innerHTML = `<i class="fa-solid ${type === 'success' ? 'fa-check' : 'fa-info'}"></i> <span>${message}</span>`;
  container.appendChild(toast);

  setTimeout(() => {
    toast.style.opacity = '0';
    setTimeout(() => toast.remove(), 300);
  }, 3500);
}

// 1. C++ Logistic Regression Category Classification
function autoDetectCategory() {
  const nameInput = document.getElementById('product-name');
  const descInput = document.getElementById('product-description');
  const catSelect = document.getElementById('product-category');
  const badgeResult = document.getElementById('category-prediction-badge');

  if (!nameInput || !nameInput.value.trim()) {
    alert('Please enter a product title first.');
    return;
  }

  const text = nameInput.value.trim() + ' ' + (descInput ? descInput.value.trim() : '');

  if (badgeResult) {
    badgeResult.innerHTML = '<span class="text-muted"><i class="fa-solid fa-spinner fa-spin"></i> Classifying via C++ Logistic Regression...</span>';
  }

  fetch('/api/products/classify-category', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ text: text })
  })
  .then(res => res.json())
  .then(data => {
    if (data.success && data.predicted_category) {
      if (catSelect) {
        for (let i = 0; i < catSelect.options.length; i++) {
          if (catSelect.options[i].text.toLowerCase() === data.predicted_category.toLowerCase()) {
            catSelect.selectedIndex = i;
            break;
          }
        }
      }
      if (badgeResult) {
        badgeResult.innerHTML = `<span class="badge bg-success-subtle text-success border border-success px-2 py-1"><i class="fa-solid fa-brain me-1"></i> C++ AI Predicted: ${data.predicted_category} (${Math.round(data.confidence * 100)}% confidence)</span>`;
      }
    } else {
      if (badgeResult) {
        badgeResult.innerHTML = `<span class="badge bg-secondary-subtle text-muted border px-2 py-1">${data.message || 'Model requires more categorized training samples.'}</span>`;
      }
    }
  })
  .catch(err => {
    console.error('Classification error:', err);
    if (badgeResult) badgeResult.innerHTML = '<span class="text-danger">Request failed</span>';
  });
}

// 2. C++ Random Forest Price Predictor
function estimateMarketPrice() {
  const catSelect = document.getElementById('product-category');
  const brandInput = document.getElementById('product-brand');
  const descInput = document.getElementById('product-description');
  const priceInput = document.getElementById('product-price');
  const origPriceInput = document.getElementById('product-orig-price');
  const badgeResult = document.getElementById('price-prediction-badge');

  const catId = catSelect ? parseInt(catSelect.value) || 1 : 1;
  const brandName = brandInput ? brandInput.value.trim() : 'Generic';
  const descText = descInput ? descInput.value.trim() : '';

  if (badgeResult) {
    badgeResult.innerHTML = '<span class="text-muted"><i class="fa-solid fa-spinner fa-spin"></i> Computing estimate via C++ Random Forest Regressor...</span>';
  }

  fetch('/api/products/predict-price', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      category_id: catId,
      brand_id: 1,
      original_price: origPriceInput && origPriceInput.value ? parseFloat(origPriceInput.value) : 1000.0,
      discount_percent: 10,
      rating: 4.2,
      description_length: descText.length
    })
  })
  .then(res => res.json())
  .then(data => {
    if (data.success && data.predicted_price) {
      if (priceInput && (!priceInput.value || parseFloat(priceInput.value) === 0)) {
        priceInput.value = data.predicted_price;
      }
      if (badgeResult) {
        badgeResult.innerHTML = `<span class="badge bg-primary-subtle text-primary border border-primary px-2 py-1"><i class="fa-solid fa-tree me-1"></i> Random Forest Fair Price: ₹${data.predicted_price.toLocaleString('en-IN')}</span>`;
      }
    } else {
      if (badgeResult) {
        badgeResult.innerHTML = `<span class="badge bg-secondary-subtle text-muted border px-2 py-1">${data.message || 'Requires minimum 5 pricing samples to train.'}</span>`;
      }
    }
  })
  .catch(err => {
    console.error('Price estimation error:', err);
    if (badgeResult) badgeResult.innerHTML = '<span class="text-danger">Estimation request failed</span>';
  });
}

// 3. Process Highest Priority Order via C++ Max-Heap Priority Queue
function processNextPriorityOrder() {
  fetch('/api/admin/orders/process-priority', { method: 'POST' })
    .then(r => r.json())
    .then(data => {
      if (data.success) {
        showAdminToast(`Processed order #${data.order_number} (Priority Score: ${data.priority_score})!`, 'success');
        // Refresh orders and priority queue tables
        if (typeof loadAdminOrders === 'function') loadAdminOrders();
        if (typeof loadPriorityQueueSnapshot === 'function') loadPriorityQueueSnapshot();
        if (typeof loadDashboardStats === 'function') loadDashboardStats();
      } else {
        showAdminToast(data.message || 'Priority queue is empty.', 'info');
      }
    })
    .catch(err => {
      console.error('Priority queue error:', err);
      showAdminToast('Failed to process priority order', 'error');
    });
}

// 4. Update Order Status
function updateOrderStatus(orderId, newStatus) {
  fetch(`/api/admin/orders/${orderId}/status`, {
    method: 'PUT',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ status: newStatus })
  })
  .then(r => r.json())
  .then(d => {
    if (d.success) {
      showAdminToast(`Order status updated to '${newStatus}'`, 'success');
    } else {
      showAdminToast(d.message || 'Failed to update order status', 'error');
    }
  });
}

// 5. Delete Customer Review (Admin Moderation)
function deleteCustomerReview(reviewId, rowElement) {
  if (!confirm('Are you sure you want to remove this customer review?')) return;

  fetch(`/api/admin/reviews/${reviewId}`, { method: 'DELETE' })
    .then(r => r.json())
    .then(d => {
      if (d.success) {
        showAdminToast('Review removed from storefront', 'success');
        if (rowElement) rowElement.remove();
      } else {
        showAdminToast(d.message || 'Failed to delete review', 'error');
      }
    });
}

// 6. Admin Sign Out
function adminLogout() {
  fetch('/api/admin/logout', { method: 'POST' })
    .then(() => {
      window.location.href = '/admin/login';
    });
}
