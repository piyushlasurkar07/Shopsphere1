/**
 * ============================================================================
 * ShopSphere Main Client-side Script
 * Connects directly to Drogon C++ REST APIs
 * Features: Live Trie Search Autocomplete, Cart, Wishlist, Realtime Sentiment Preview
 * ============================================================================
 */

// Global Toast Notification Helper
function showToast(message, type = 'success') {
  let toastContainer = document.getElementById('toast-container');
  if (!toastContainer) {
    toastContainer = document.createElement('div');
    toastContainer.id = 'toast-container';
    toastContainer.style.position = 'fixed';
    toastContainer.style.bottom = '24px';
    toastContainer.style.right = '24px';
    toastContainer.style.zIndex = '9999';
    toastContainer.style.display = 'flex';
    toastContainer.style.flexDirection = 'column';
    toastContainer.style.gap = '10px';
    document.body.appendChild(toastContainer);
  }

  const toast = document.createElement('div');
  const bg = type === 'success' ? '#10b981' : (type === 'error' ? '#ef4444' : '#2563eb');
  const icon = type === 'success' ? 'fa-circle-check' : (type === 'error' ? 'fa-circle-xmark' : 'fa-circle-info');
  
  toast.style.background = '#ffffff';
  toast.style.borderLeft = `5px solid ${bg}`;
  toast.style.boxShadow = '0 10px 15px -3px rgba(0, 0, 0, 0.1), 0 4px 6px -2px rgba(0, 0, 0, 0.05)';
  toast.style.padding = '12px 18px';
  toast.style.borderRadius = '8px';
  toast.style.fontSize = '0.9rem';
  toast.style.fontWeight = '500';
  toast.style.color = '#0f172a';
  toast.style.display = 'flex';
  toast.style.alignItems = 'center';
  toast.style.gap = '10px';
  toast.style.minWidth = '260px';
  toast.style.transition = 'all 0.3s ease';
  toast.innerHTML = `<i class="fa-solid ${icon}" style="color: ${bg}; font-size: 1.1rem;"></i> <span>${message}</span>`;

  toastContainer.appendChild(toast);

  setTimeout(() => {
    toast.style.opacity = '0';
    toast.style.transform = 'translateY(10px)';
    setTimeout(() => toast.remove(), 300);
  }, 3200);
}

// Update Header Cart & Wishlist Badges
function updateCounters() {
  fetch('/api/cart')
    .then(r => r.json())
    .then(d => {
      const badges = document.querySelectorAll('.cart-count-badge');
      badges.forEach(b => {
        b.textContent = d.total_items || 0;
        b.style.display = (d.total_items && d.total_items > 0) ? 'inline-block' : 'none';
      });
    }).catch(() => {});

  fetch('/api/wishlist')
    .then(r => r.json())
    .then(d => {
      if (d.success && d.items) {
        const badges = document.querySelectorAll('.wishlist-count-badge');
        badges.forEach(b => {
          b.textContent = d.items.length || 0;
          b.style.display = d.items.length > 0 ? 'inline-block' : 'none';
        });
      }
    }).catch(() => {});
}

// Add to Cart via C++ Drogon REST API
function addToCart(productId, quantity = 1) {
  fetch('/api/cart/add', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ product_id: parseInt(productId), quantity: parseInt(quantity) })
  })
  .then(res => res.json())
  .then(data => {
    if (data.success) {
      showToast(data.message || 'Item added to your shopping cart!', 'success');
      updateCounters();
    } else {
      showToast(data.message || 'Unable to add item to cart', 'error');
    }
  })
  .catch(err => {
    console.error('Cart error:', err);
    showToast('Failed to update cart', 'error');
  });
}

// Wishlist Toggle
function toggleWishlist(productId, btnElement) {
  fetch('/api/wishlist/toggle', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ product_id: parseInt(productId) })
  })
  .then(res => res.json())
  .then(data => {
    if (data.success) {
      if (btnElement) {
        const icon = btnElement.querySelector('i');
        if (data.in_wishlist) {
          btnElement.classList.add('active');
          if (icon) {
            icon.classList.remove('fa-regular');
            icon.classList.add('fa-solid');
            icon.style.color = '#ef4444';
          }
          showToast('Added to your Wishlist', 'success');
        } else {
          btnElement.classList.remove('active');
          if (icon) {
            icon.classList.remove('fa-solid');
            icon.classList.add('fa-regular');
            icon.style.color = '';
          }
          showToast('Removed from Wishlist', 'info');
        }
      }
      updateCounters();
    } else {
      showToast(data.message || 'Please sign in to manage your wishlist', 'error');
      if (data.message && data.message.includes('Sign in')) {
        setTimeout(() => { window.location.href = '/login'; }, 1000);
      }
    }
  })
  .catch(err => {
    console.error('Wishlist error:', err);
  });
}

// Product Detail Image Switcher
function switchMainImage(thumbElement, imgUrl) {
  const mainImg = document.getElementById('main-product-image');
  if (mainImg) {
    mainImg.src = imgUrl;
    document.querySelectorAll('.gallery-thumb').forEach(el => el.classList.remove('active'));
    if (thumbElement) thumbElement.classList.add('active');
  }
}

// C++ Trie-Based Live Autocomplete Search Handler
function setupTrieSearchAutocomplete() {
  const searchInput = document.querySelector('.search-input');
  if (!searchInput) return;

  const container = searchInput.parentElement;
  if (!container) return;
  container.style.position = 'relative';

  let dropdown = document.createElement('div');
  dropdown.className = 'trie-autocomplete-dropdown shadow-lg rounded-3 border-0';
  dropdown.style.cssText = 'position: absolute; top: calc(100% + 4px); left: 0; right: 0; background: #fff; z-index: 1000; display: none; max-height: 320px; overflow-y: auto; border: 1px solid #e2e8f0;';
  container.appendChild(dropdown);

  let debounceTimer = null;

  searchInput.addEventListener('input', function() {
    const query = this.value.trim();
    clearTimeout(debounceTimer);

    if (query.length < 2) {
      dropdown.style.display = 'none';
      dropdown.innerHTML = '';
      return;
    }

    debounceTimer = setTimeout(() => {
      fetch(`/api/search/suggestions?q=${encodeURIComponent(query)}`)
        .then(r => r.json())
        .then(data => {
          if (data.success && data.suggestions && data.suggestions.length > 0) {
            let html = '<div class="py-2">';
            html += '<div class="px-3 py-1 text-uppercase text-muted" style="font-size: 0.7rem; font-weight: 700; letter-spacing: 0.05em;"><i class="fa-solid fa-bolt text-warning me-1"></i> Trie Autocomplete Suggestions</div>';
            data.suggestions.forEach(item => {
              html += `<a href="/products?q=${encodeURIComponent(item.term)}" class="d-flex align-items-center justify-content-between px-3 py-2 text-decoration-none text-dark border-bottom border-light trie-item" style="transition: background 0.15s ease;">
                <span><i class="fa-solid fa-magnifying-glass text-muted me-2" style="font-size: 0.8rem;"></i> ${item.term}</span>
                <span class="badge bg-light text-muted rounded-pill" style="font-size: 0.7rem;">${item.frequency} matches</span>
              </a>`;
            });
            html += '</div>';
            dropdown.innerHTML = html;
            dropdown.style.display = 'block';

            // Hover effect
            dropdown.querySelectorAll('.trie-item').forEach(el => {
              el.addEventListener('mouseenter', () => el.style.background = '#f8fafc');
              el.addEventListener('mouseleave', () => el.style.background = '#ffffff');
            });
          } else {
            dropdown.style.display = 'none';
          }
        })
        .catch(() => { dropdown.style.display = 'none'; });
    }, 180);
  });

  document.addEventListener('click', function(e) {
    if (!container.contains(e.target)) {
      dropdown.style.display = 'none';
    }
  });
}

// C++ VADER Sentiment Preview on Review Writing
function setupSentimentAnalyzer() {
  const reviewText = document.getElementById('review-text');
  const sentimentPreview = document.getElementById('sentiment-preview-badge');
  if (!reviewText || !sentimentPreview) return;

  let debounceTimer = null;
  reviewText.addEventListener('input', function() {
    const text = this.value.trim();
    clearTimeout(debounceTimer);

    if (text.length < 5) {
      sentimentPreview.innerHTML = '';
      return;
    }

    debounceTimer = setTimeout(() => {
      fetch('/api/reviews/analyze', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ text: text })
      })
      .then(r => r.json())
      .then(d => {
        if (d.success) {
          let badgeColor = d.sentiment === 'Positive' ? 'success' : (d.sentiment === 'Negative' ? 'danger' : 'secondary');
          let icon = d.sentiment === 'Positive' ? 'fa-face-smile' : (d.sentiment === 'Negative' ? 'fa-face-frown' : 'fa-face-meh');
          
          let aspects = '';
          if (d.aspect_quality > 0) aspects += '<span class="badge bg-light text-dark me-1 border">Quality: Positive</span>';
          if (d.aspect_delivery > 0) aspects += '<span class="badge bg-light text-dark me-1 border">Delivery: Positive</span>';
          if (d.aspect_price > 0) aspects += '<span class="badge bg-light text-dark border">Value: Positive</span>';

          sentimentPreview.innerHTML = `
            <div class="d-flex align-items-center gap-2 mt-2">
              <span class="badge bg-${badgeColor}-subtle text-${badgeColor} border border-${badgeColor} px-2 py-1">
                <i class="fa-solid ${icon} me-1"></i> Sentiment: ${d.sentiment} (${d.score > 0 ? '+' : ''}${d.score})
              </span>
              ${aspects}
            </div>
          `;
        }
      })
      .catch(() => {});
    }, 250);
  });
}

// Interactive Review Stars
function setupStarRating() {
  const starInputs = document.querySelectorAll('.star-rating-select i');
  const ratingValueInput = document.getElementById('selected-star-value');

  if (starInputs.length > 0 && ratingValueInput) {
    starInputs.forEach(star => {
      star.addEventListener('click', function() {
        const val = parseInt(this.getAttribute('data-value'));
        ratingValueInput.value = val;
        starInputs.forEach(s => {
          const sVal = parseInt(s.getAttribute('data-value'));
          if (sVal <= val) {
            s.classList.remove('fa-regular');
            s.classList.add('fa-solid');
            s.style.color = '#f59e0b';
          } else {
            s.classList.remove('fa-solid');
            s.classList.add('fa-regular');
            s.style.color = '#cbd5e1';
          }
        });
      });
    });
  }
}

// Check Current User Auth Status
function checkAuthStatus() {
  fetch('/api/auth/me')
    .then(r => r.json())
    .then(data => {
      const userGreeting = document.getElementById('user-greeting-name');
      const authLinks = document.getElementById('nav-auth-links');
      if (data.authenticated) {
        if (userGreeting) userGreeting.textContent = data.full_name ? data.full_name.split(' ')[0] : 'User';
        if (authLinks) {
          authLinks.innerHTML = `
            <li class="px-3 py-2 border-bottom">
              <div class="fw-bold text-dark">${data.full_name}</div>
              <small class="text-muted">${data.email}</small>
            </li>
            <li><a class="dropdown-item py-2" href="/profile"><i class="fa-regular fa-id-badge me-2 text-primary"></i>Your Profile</a></li>
            <li><a class="dropdown-item py-2" href="/orders"><i class="fa-solid fa-box me-2 text-primary"></i>Your Orders</a></li>
            <li><a class="dropdown-item py-2" href="/wishlist"><i class="fa-regular fa-heart me-2 text-danger"></i>Your Wishlist</a></li>
            <li><hr class="dropdown-divider"></li>
            <li><a class="dropdown-item py-2 text-danger" href="javascript:void(0)" onclick="logout()"><i class="fa-solid fa-arrow-right-from-bracket me-2"></i>Sign Out</a></li>
          `;
        }
      } else {
        if (userGreeting) userGreeting.textContent = 'Sign In';
        if (authLinks) {
          authLinks.innerHTML = `
            <li class="p-3 text-center">
              <a href="/login" class="btn btn-primary w-100 fw-bold mb-2">Sign In</a>
              <small class="text-muted">New customer? <a href="/register" class="text-primary fw-semibold">Start here</a></small>
            </li>
            <li><hr class="dropdown-divider my-0"></li>
            <li><a class="dropdown-item py-2" href="/orders"><i class="fa-solid fa-box me-2 text-secondary"></i>Your Orders</a></li>
            <li><a class="dropdown-item py-2" href="/wishlist"><i class="fa-regular fa-heart me-2 text-secondary"></i>Wishlist</a></li>
          `;
        }
      }
    }).catch(() => {});
}

// User Sign Out
function logout() {
  fetch('/api/auth/logout', { method: 'POST' })
    .then(() => {
      showToast('Logged out successfully', 'info');
      setTimeout(() => { window.location.href = '/'; }, 500);
    });
}

// Initialization on DOM Loaded
document.addEventListener('DOMContentLoaded', () => {
  setupTrieSearchAutocomplete();
  setupSentimentAnalyzer();
  setupStarRating();
  checkAuthStatus();
  updateCounters();
});
