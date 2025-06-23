import matplotlib.pyplot as plt

# Sample data points: (q, m_max)
q_values = [257, 1153, 19457, 36097, 50177, 100609, 315521, 627329, 700673, 708481, 716033, 724481]
m_max_values = [2, 3, 16, 22, 26, 37, 66, 93, 98, 99, 99, 100]

# Plotting
plt.figure(figsize=(8, 5))
plt.plot(q_values, m_max_values, marker='o', linestyle='-', color='royalblue', label='Max m vs q')

# Labels and title
plt.xlabel('Modulus q', fontsize=12)
plt.ylabel('Maximum m (before protocol breaks)', fontsize=12)
plt.title('q vs m_max for AVP Protocol', fontsize=14)
plt.grid(True)
plt.legend()

# Show plot
plt.tight_layout()
plt.show()
